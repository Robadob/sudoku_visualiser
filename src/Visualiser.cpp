#include "Visualiser.h"

#include <thread>

#include "util/cuda.h"
#include "util/fonts.h"
#include "util/MouseButtonState.h"
#include "sudoku/Board.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/easing.hpp>


#define FOVY 60.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_ROLL 0.01f
#define ONE_SECOND_MS 1000
#define VSYNC 1
#define MAX_FRAMERATE 60

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

Visualiser::Visualiser()
    : hud(std::make_shared<HUD>(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT))
    , camera()
    , isInitialised(false)
    , continueRender(false)
    , msaaState(true)
    , windowTitle("Sudoku Visualiser")
    , windowDims(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)
    , fpsDisplay(nullptr)
    , sudoku_board(std::make_shared<Board>(this)) {
    this->isInitialised = this->init();
    BackBuffer::setClear(true, glm::vec3(0.8f));
    {
        fpsDisplay = std::make_shared<Text>("", 10, glm::vec3(0), fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str());
        fpsDisplay->setUseAA(false);
        hud->add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0), INT_MAX);
    }
    {
        notificationDisplay = std::make_shared<Text>("", 50, glm::vec3(0), fonts::findFont({"Comic Sans MS"}, fonts::GenericFontFamily::SANS).c_str());
        notificationDisplay->setVisible(true);
        notificationDisplay->setColor(glm::vec3(0));
        notificationDisplay->setBackgroundColor(glm::vec4(0.8f));
        notificationDisplay->setUseAA(true);
        hud->add(notificationDisplay, HUD::AnchorV::Center, HUD::AnchorH::Center, glm::ivec2(0), INT_MAX);
    }
    {  // L"Test✓✓✔✅√☒☑☐✕✗✘✖❌"
        constraintsOptions = std::make_shared<ToggleList>(std::vector<std::string>({"Test", "Item 2"}));
        constraintsOptions->setVisible(true);
        // constraintsOptions->setColor(glm::vec3(0));
        // constraintsOptions->setBackgroundColor(glm::vec4(0.8f));
        // constraintsOptions->setUseAA(true);
        hud->add(constraintsOptions, HUD::AnchorV::North, HUD::AnchorH::East, glm::ivec2(0), 1);
    }
    hud->add(sudoku_board->getOverlay(DEFAULT_WINDOW_HEIGHT), HUD::AnchorV::Center, HUD::AnchorH::Center, glm::ivec2(0), 0);
}
Visualiser::~Visualiser() {
    this->close();
}
void Visualiser::start() {
    // Only execute if background thread is not active
    if (!isRunning()) {
        if (this->background_thread) {
            // Async window was closed via cross, so thread still exists
            join();  // This kills the thread properly
        }
        // Clear the context from current thread, otherwise we cant move it to background thread
        SDL_GL_MakeCurrent(this->window, NULL);
        SDL_DestroyWindow(this->window);
        // Launch render loop in a new thread
        this->background_thread = new std::thread(&Visualiser::run, this);
    } else {
        printf("Already running! Call quit() to close it first!\n");
    }
}
void Visualiser::join() {
    // Only join if background thread exists, and we are not executing in it
    if (this->background_thread && std::this_thread::get_id() != this->background_thread->get_id()) {
        // Wait for thread to exit
        this->background_thread->join();
        delete this->background_thread;
        this->background_thread = nullptr;
        // Recreate hidden window in current thread, so context is stable
        SDL_GL_MakeCurrent(this->window, NULL);
        SDL_DestroyWindow(this->window);
        this->window = SDL_CreateWindow(
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS
        SDL_GL_MakeCurrent(this->window, this->context);
    }
}
void Visualiser::stop() {
    printf("Visualiser::stop()\n");
    this->continueRender = false;
}
void Visualiser::run() {
    if (!this->isInitialised) {
        printf("Visulisation not initialised yet.\n");
    // } else if (!this->scene) {
    //     printf("Scene not yet set.\n");
    } else {
        // Recreate window in current thread (else IO fails)
        if (this->window) {
            SDL_GL_MakeCurrent(this->window, NULL);
            SDL_DestroyWindow(this->window);
        }
        this->window = SDL_CreateWindow(
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS
        if (!this->window) {
            printf("Window failed to init.\n");
        } else {
            // Update window title to whatever default mode is
            setWindowTitleMode();
            int err = SDL_GL_MakeCurrent(this->window, this->context);
            if (err != 0) {
                THROW VisAssert("Visualiser::run(): SDL_GL_MakeCurrent failed!\n", SDL_GetError());
            }
            GL_CHECK();
            this->resizeWindow();
            GL_CHECK();
            SDL_StartTextInput();
            this->continueRender = true;
            while (this->continueRender) {
                const auto frameStart = std::chrono::high_resolution_clock::now();
                //  Update the fps in the window title
                this->updateFPS();
                // Process notification removal
                {
                    if (notification_timout) {
                        const unsigned int diff = glm::clamp<unsigned int>(notification_timout - currentTime, 0, notification_millis);
                        const float modifier = glm::cubicEaseOut(diff/static_cast<float>(notification_millis));
                        glm::vec4 fg = notificationDisplay->getColor();
                        glm::vec4 bg = notificationDisplay->getBackgroundColor();
                        fg.a = 1.0f * modifier;
                        bg.a = 0.8f * modifier;
                        notificationDisplay->setColor(fg);
                        notificationDisplay->setBackgroundColor(bg);
                        if (notification_timout < currentTime) {
                            // Due to use of uint to calculate diff, colors are always reset to max alpha on exit
                            notification_timout = 0;
                            notificationDisplay->setVisible(false);
                        }
                    }
                }
                if (this->sudoku_board->hasOverlay())
                    this->sudoku_board->getOverlay()->update();
                this->render();
                //  update the screen
                SDL_GL_SwapWindow(window);
#ifdef MAX_FRAMERATE
                {
                    // Enforce framerate cap
                    // (This prevents the possibility of driver vsync causing empty spinning till vsync tick, hence reduces processor usage)
                    // This algorithm is the best compromise to work alongside vsync (on my personal machine), will run slightly fast if vsync disabled
                    // Likely to perform differently with non-windows OS, or non-nvidia GPU
                    const auto frameEnd = std::chrono::high_resolution_clock::now();
                    const auto frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart);
                    const double FPS_TIME = static_cast<double>(ONE_SECOND_MS)/(MAX_FRAMERATE);
                    if (frameTime.count() < FPS_TIME - 1) {
                        auto delay = std::chrono::duration<double, std::milli>(FPS_TIME - 1) - frameTime;
                        std::this_thread::sleep_for(delay);
                    } else if (frameTime.count() > FPS_TIME) {
                        auto delay = std::chrono::duration<double, std::milli>(2 * FPS_TIME - 1) - frameTime;
                        std::this_thread::sleep_for(delay);
                    }
                }
#endif
            }
            SDL_StopTextInput();
            // Release mouse lock
            if (SDL_GetRelativeMouseMode()) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            // Un-pause the simulation if required.
            if (this->pause_guard) {
                delete this->pause_guard;
                this->pause_guard = nullptr;
            }
            // Hide window
            SDL_HideWindow(window);
            // New, might not be required
            // SDL_DestroyWindow(this->window);
            // this->window = nullptr;
        }
    }
}
void Visualiser::render() {
    static MouseButtonState last_buttons;
    // Static fn var for tracking the time to send to scene->update()
    static unsigned int updateTime = 0;
    const unsigned int t_updateTime = SDL_GetTicks();
    // If the program runs for over ~49 days, the return value of SDL_GetTicks() will wrap
    const unsigned int frameTime = t_updateTime < updateTime ? (t_updateTime + (UINT_MAX - updateTime)) : t_updateTime - updateTime;
    updateTime = t_updateTime;
    SDL_Event e;
    //  Handle continuous key presses (movement)
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    // float speed = modelConfig.cameraSpeed[0];
    // const float distance = speed * static_cast<float>(frameTime);
    // if (state[SDL_SCANCODE_W]) {
    //     this->camera->move(distance);
    // }
    // if (state[SDL_SCANCODE_A]) {
    //     this->camera->strafe(-distance);
    // }
    // if (state[SDL_SCANCODE_S]) {
    //     this->camera->move(-distance);
    // }
    // if (state[SDL_SCANCODE_D]) {
    //     this->camera->strafe(distance);
    // }
    // if (state[SDL_SCANCODE_Q]) {
    //     this->camera->roll(-DELTA_ROLL);
    // }
    // if (state[SDL_SCANCODE_E]) {
    //     this->camera->roll(DELTA_ROLL);
    // }
    // if (state[SDL_SCANCODE_SPACE]) {
    //     this->camera->ascend(distance);
    // }
    // if (state[SDL_SCANCODE_LCTRL]) {  // Ctrl now moves slower
    //     this->camera->ascend(-distance);
    // }

    //  handle each event on the queue
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            continueRender = false;
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                resizeWindow();
            break;
        case SDL_KEYDOWN: {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            this->handleKeypress(e.key.keysym.sym, state, x, y);
        }
        break;
        // case SDL_MOUSEWHEEL:
        // break;
        case SDL_MOUSEMOTION: {
            // this->handleMouseMove(e.motion.xrel, e.motion.yrel);
            int x = 0;
            int y = 0;
            unsigned int button_state = SDL_GetMouseState(&x, &y);
            if (button_state) {
                memcpy(&last_buttons, &button_state, sizeof(int));
                hud->handleMouseDrag(x, y, last_buttons);
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            // this->toggleMouseMode();
            int x = 0;
            int y = 0;
            unsigned int button_state = SDL_GetMouseState(&x, &y);
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                memcpy(&last_buttons, &button_state, sizeof(int));
                hud->handleMouseDown(x, y, last_buttons);
            } else {
                hud->handleMouseUp(x, y, last_buttons);
                memcpy(&last_buttons, &button_state, sizeof(int));
            }
            break;
        }
        }
    }
    //  render
    BackBuffer::useStatic();

    GL_CALL(glViewport(0, 0, windowDims.x, windowDims.y));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    this->hud->render();

    GL_CHECK();
}
bool Visualiser::isRunning() const {
    return continueRender;
}

//  Items taken from sdl_exp
bool Visualiser::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialise SDL: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    //  Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    // Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    this->window = SDL_CreateWindow(
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowDims.x,
        this->windowDims.y,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS

    if (!this->window) {
        printf("Window failed to init.\n");
    } else {
        SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
        SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);

        //  Get context
        this->context = SDL_GL_CreateContext(window);

        // Enable VSync
        int swapIntervalResult = SDL_GL_SetSwapInterval(VSYNC);
        if (swapIntervalResult == -1) {
            printf("Swap Interval Failed: %s\n", SDL_GetError());
        }

        GLEW_INIT();

        //  Setup gl stuff
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glCullFace(GL_BACK));
        GL_CALL(glEnable(GL_CULL_FACE));
        GL_CALL(glShadeModel(GL_SMOOTH));
        GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        GL_CALL(glBlendEquation(GL_FUNC_ADD));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        BackBuffer::setClear(true, glm::vec3(0));  // Clear to black
        setMSAA(this->msaaState);

        //  Setup the projection matrix
        this->resizeWindow();
        GL_CHECK();
        return true;
    }
    return false;
}
void Visualiser::setMSAA(bool state) {
    this->msaaState = state;
    if (this->msaaState)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}
void Visualiser::resizeWindow() {
    //  Use the sdl drawable size
    {
        glm::ivec2 tDims;
        SDL_GL_GetDrawableSize(this->window, &tDims.x, &tDims.y);
        this->windowDims = tDims;
    }
    //  Get the view frustum using GLM. Alternatively glm::perspective could be used.
    this->projMat = glm::perspectiveFov<float>(
        glm::radians(FOVY),
        static_cast<float>(this->windowDims.x),
        static_cast<float>(this->windowDims.y),
        0.05f, 5000);
    //  Notify other elements
    this->hud->resizeWindow(this->windowDims);
    //  if (this->scene)
    //     this->scene->_resize(this->windowDims);  //  Not required unless we use multipass framebuffering
    resizeBackBuffer(this->windowDims);
}
void Visualiser::deallocateGLObjects() {
    sudoku_board->killOverlay();
    fpsDisplay.reset();
    notificationDisplay.reset();
    constraintsOptions.reset();
    this->hud->clear();
}

void Visualiser::close() {
    continueRender = false;
    if (this->pause_guard) {
        delete this->pause_guard;
        this->pause_guard = nullptr;
    }
    if (this->background_thread) {
        this->background_thread->join();
        delete this->background_thread;
    }
    //  This really shouldn't run if we're not the host thread, but we don't manage the render loop thread
    if (this->window != nullptr) {
        SDL_GL_MakeCurrent(this->window, this->context);
        // Delete objects before we delete the GL context!
        deallocateGLObjects();
        SDL_DestroyWindow(this->window);
        this->window = nullptr;
    }
    if (this->context != nullptr) {
        SDL_GL_DeleteContext(this->context);
    }
    SDL_Quit();
}
void Visualiser::handleMouseMove(int x, int y) {
    // if (SDL_GetRelativeMouseMode()) {
    //     this->camera->turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
    // }
}
void Visualiser::handleKeypress(SDL_Keycode keycode, const Uint8 *keyboard_state, int /*x*/, int /*y*/) {
    // Pass key events to the scene and skip handling if false is returned
    // if (scene && !scene->_keypress(keycode, x, y))
    //     return;
    switch (keycode) {
    case SDLK_ESCAPE:
        continueRender = false;
        break;
    case SDLK_F11:
        this->toggleFullScreen();
        break;
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F8:
        if (this->fpsDisplay)
            this->fpsDisplay->setVisible(!this->fpsDisplay->getVisible());
        break;
    case SDLK_F5:
        // if (this->scene)
        //     this->scene->_reload();
        this->hud->reload();
        break;
    case SDLK_m: {
        int m = this->sudoku_board->getMode() + 1;
        m = m == End ? 0 : m;
        this->sudoku_board->setMode(Mode(m));
        // Notify user of the change
        sendNotification(to_string(Mode(m)));
        // Update window title
        std::string newTitle = "Sudoku Visualiser - ";
        newTitle += to_string(Mode(m));
        setWindowTitleMode();
    }
    case SDLK_p: {
        if (this->pause_guard) {
            delete pause_guard;
            pause_guard = nullptr;
        } else {
            pause_guard = new std::lock_guard<std::mutex>(render_buffer_mutex);
        }
        break;
    }
    // Sudoku Arrow
    case SDLK_LEFT:  sudoku_board->shiftSelectedCell(-1, +0); break;
    case SDLK_RIGHT: sudoku_board->shiftSelectedCell(+1, +0); break;
    case SDLK_UP:    sudoku_board->shiftSelectedCell(+0, -1); break;
    case SDLK_DOWN:  sudoku_board->shiftSelectedCell(+0, +1); break;
    default:
        if (this->sudoku_board) {
            const bool shift_state = keyboard_state[SDL_SCANCODE_LSHIFT] || keyboard_state[SDL_SCANCODE_RSHIFT];
            const bool ctrl_state = keyboard_state[SDL_SCANCODE_LCTRL] || keyboard_state[SDL_SCANCODE_RCTRL];
            const bool alt_state = keyboard_state[SDL_SCANCODE_LALT] || keyboard_state[SDL_SCANCODE_RALT];
            // Convert keycode to number [0-9] (0 and backspace, acts as clear)
            // Set marks ctrl/shift + number [0-9]
            // Ctrl z/y, edit undo/redo
            sudoku_board->handleKeyPress(keycode, shift_state, ctrl_state, alt_state);
        }
        break;
    }
}
bool Visualiser::isFullscreen() const {
    //  Use window borders as a toggle to detect fullscreen.
    return (SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS;
}
void Visualiser::toggleFullScreen() {
    if (this->isFullscreen()) {
        //  Update the window using the stored windowBounds
        SDL_SetWindowBordered(this->window, SDL_TRUE);
        SDL_SetWindowSize(this->window, this->windowedBounds.w, this->windowedBounds.h);
        SDL_SetWindowPosition(this->window, this->windowedBounds.x, this->windowedBounds.y);
    } else {
        //  Store the windowedBounds for later
        SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
        SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);
        //  Get the window bounds for the current screen
        int displayIndex = SDL_GetWindowDisplayIndex(this->window);
        SDL_Rect displayBounds;
        SDL_GetDisplayBounds(displayIndex, &displayBounds);
        //  Update the window
        SDL_SetWindowBordered(this->window, SDL_FALSE);
        SDL_SetWindowPosition(this->window, displayBounds.x, displayBounds.y);
        SDL_SetWindowSize(this->window, displayBounds.w, displayBounds.h);
    }
    // this->resizeWindow(); will be triggered by SDL_WINDOWEVENT_SIZE_CHANGED
}
void Visualiser::toggleMouseMode() {
    if (SDL_GetRelativeMouseMode()) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
}
void Visualiser::updateFPS() {
    //  Update the current time
    this->currentTime = SDL_GetTicks();
    //  Update frame counter
    this->frameCount += 1;
    //  If it's been more than a second, do something.
    if (this->currentTime > this->previousTime + ONE_SECOND_MS) {
        //  Calculate average fps.
        double fps = this->frameCount / static_cast<double>(this->currentTime - this->previousTime) * ONE_SECOND_MS;
        // Update the FPS string
        if (this->fpsDisplay)
            this->fpsDisplay->setString("%.3f fps", fps);
        //  reset values;
        this->previousTime = this->currentTime;
        this->frameCount = 0;
    }
}
//  Overrides
unsigned Visualiser::getWindowWidth() const {
    return windowDims.x;
}
unsigned Visualiser::getWindowHeight() const {
    return windowDims.y;
}
glm::uvec2 Visualiser::getWindowDims() const {
    return windowDims;
}
const glm::mat4 * Visualiser::getProjectionMatPtr() const {
    return &projMat;
}
glm::mat4 Visualiser::getProjectionMat() const {
    return projMat;
}
std::shared_ptr<const Camera> Visualiser::getCamera() const {
    return camera;
}
std::weak_ptr<HUD> Visualiser::getHUD() {
    return hud;
}
const char *Visualiser::getWindowTitle() const {
    return windowTitle;
}
void Visualiser::setWindowTitle(const char *_windowTitle) {
    SDL_SetWindowTitle(window, _windowTitle);
    windowTitle = _windowTitle;
}
void Visualiser::setWindowTitleMode() {
    // Update window title
    std::string newTitle = "Sudoku Visualiser - ";
    newTitle += to_string(Mode(this->sudoku_board->getMode()));
    setWindowTitle(newTitle.c_str());
}
void Visualiser::sendNotification(const std::string &notification, unsigned int timeout) {
    notificationDisplay->setString(notification.c_str());
    notificationDisplay->setVisible(true);
    notification_timout = currentTime + timeout;
    notification_millis = timeout;
}
