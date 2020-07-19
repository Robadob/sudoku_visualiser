#ifndef SRC_VISUALISER_H_
#define SRC_VISUALISER_H_

#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <utility>
#include <thread>
#include <list>

#include <SDL.h>
#undef main  // SDL breaks the regular main entry point, this fixes
#define GLM_FORCE_NO_CTOR_INIT
#include <glm/glm.hpp>

#include "interface/Viewport.h"
#include "camera/NoClipCamera.h"
#include "HUD.h"
#include "Text.h"

#include "config/ModelConfig.h"
#include "config/AgentStateConfig.h"
#include "Draw.h"
#include "Entity.h"

#include "sodoku/BoardOverlay.h"

/**
 * This is the main class of the visualisation, hosting the window and render loop
 */
class Visualiser : public ViewportExt {
 public:
    Visualiser();
    ~Visualiser();
    /**
     * Starts the render loop running
     */
    void start();
    /**
     * Stop the render loop
     */
    void stop();
    /**
     * Block until the render loop has stopped
     * This expects the render loop to exit via the user closing the window (or similar)
     */
    void join();
    /**
     * Returns true if the render loop's thread exists
     */
    bool isRunning() const;

    void run();
    /**
     * A single pass of the render loop
     * Also handles keyboard/mouse IO
     */
    void render();
    /**
     * Toggles the window between borderless fullscreen and windowed states
     */
    void toggleFullScreen();
    /**
     * @return True if the window is currently full screen
     */
    bool isFullscreen() const;
    /**
     * Toggles whether the mouse is hidden and returned relative to the window
     */
    void toggleMouseMode();
    /**
     * Toggles whether Multi-Sample Anti-Aliasing should be used or not
     * @param state The desired MSAA state
     * @note Unless blocked by the active Scene the F10 key toggles MSAA at runtime
     */
    void setMSAA(bool state);
    /**
     * Provides key handling for none KEY_DOWN events of utility keys (ESC, F11, F10, F5, etc)
     * @param keycode The keypress detected
     * @param x The horizontal mouse position at the time of the KEY_DOWN event
     * @param y The vertical mouse position at the time of the KEY_DOWN event
     * @note Unsure whether the mouse position is relative to the window
     */
    void handleKeypress(SDL_Keycode keycode, int x, int y);
    /**
     * Moves the camera according to the motion of the mouse (whilst the mouse is attatched to the window via toggleMouseMode())
     * @param x The horizontal distance moved
     * @param y The vertical distance moved
     * @note This is called within the render loop
     */
    void handleMouseMove(int x, int y);
    /**
     * Initialises SDL and creates the window
     * @return Returns true on success
     * @note This method doesn't begin the render loop, use run() for that
     */
    bool init();
    /**
      * Util method which handles deallocating all objects which contains GLbuffers, shaders etc
      */
    void deallocateGLObjects();
    /**
     * Provides destruction of the object, deletes child objects, removes the GL context, closes the window and calls SDL_quit()
     */
    void close();
    /**
     * Simple implementation of an FPS counter
     * @note This is called within the render loop
     */
    void updateFPS();
    /**
     * Updates the viewport and projection matrix
     * This should be called after window resize events, or simply if the viewport needs generating
     */
    void resizeWindow();

    /**
     * Returns the window's current width
     * @note This does not account for fullscreen window size
     */
    unsigned int getWindowWidth() const override;
    /**
     * Returns the window's current height
     * @note This does not account for fullscreen window size
     */
    unsigned int getWindowHeight() const override;
    /**
     * Returns the window's current width and height
     * @note This does not account for fullscreen window size
     */
    glm::uvec2 getWindowDims() const override;
    const glm::mat4 * getProjectionMatPtr() const override;
    glm::mat4 getProjectionMat() const override;
    std::shared_ptr<const Camera> getCamera() const override;
    std::weak_ptr<HUD> getHUD() override;
    const char * getWindowTitle() const override;
    void setWindowTitle(const char *windowTitle) override;

 private:
    SDL_Window* window;
    SDL_Rect windowedBounds;
    SDL_GLContext context;
    /**
     * The HUD elements to be rendered
     */
    std::shared_ptr<HUD> hud;
    /**
     * The camera controller for the scene
     */
    std::shared_ptr<NoClipCamera> camera;
    /**
     * The projection matrix for the scene
     * @see resizeWindow()
     */
    glm::mat4 projMat;

    bool isInitialised;
    /**
     * Flag which tells Visualiser to exit the render loop
     */
    std::atomic<bool> continueRender;
    /**
     * Flag representing whether MSAA is currently enabled
     * MSAA can be toggled at runtime with F10
     * @see setMSAA(bool)
     */
    bool msaaState;
    /**
     * Current title of the visualisation window
     */
    const char* windowTitle;
    /**
     * Current dimensions of the window (does not account for fullscreen size)
     */
    glm::uvec2 windowDims;

    /**
     * Used for tracking and calculating fps
     */
    unsigned int previousTime = 0, currentTime, frameCount = 0;
    /**
     * The object which renders FPS text to the visualisation
     */
    std::shared_ptr<Text> fpsDisplay;
    /**
     * Background thread in which visualiser executes
     * (Timestep independent visualiser)
     */
    std::thread *background_thread = nullptr;
    /**
     * Mutex is required to access render buffers for thread safety
     */
    std::mutex render_buffer_mutex;
    /**
     * When this is not set to nullptr, it blocks the simulation from continuing
     */
    std::lock_guard<std::mutex> *pause_guard = nullptr;

    std::shared_ptr<BoardOverlay> sodoku_board;
};

#endif  // SRC_VISUALISER_H_
