#include "BoardOverlay.h"

#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>


#include "Board.h"
#include "shader/Shaders.h"

#include "util/warnings.h"
#include "util/fonts.h"


BoardOverlay::BoardOverlay(Board &parent, const unsigned int &width_height)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SUDOKU_BOARD))
    , board(parent)
    , tex(std::make_shared<BoardTex>(this)) {
    setClickable(true);
    // Preload all the glyphs we will use
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        THROW FontLoadingError("An unexpected error occurred whilst initialising FreeType: %i\n", error);
    }
    const std::string fontFile = fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS);
    // Value Font
    error = FT_New_Face(library, fontFile.c_str(), 0, &value_font);
    if (error) {
        THROW FontLoadingError("An unexpected error occurred whilst loading font file %s: %i\n", fontFile.c_str(), error);
    }
    // Mark Font
    error = FT_New_Face(library, fontFile.c_str(), 0, &mark_font);
    if (error) {
        THROW FontLoadingError("An unexpected error occurred whilst loading font file %s: %i\n", fontFile.c_str(), error);
    }
    // Setup board and shaders
    scaleBoard(width_height);
    getShaders()->addStaticUniform("_col", glm::value_ptr(this->color), 4);
    getShaders()->addStaticUniform("_backCol", glm::value_ptr(this->background_color), 4);
    getShaders()->addStaticUniform("_selCol", glm::value_ptr(this->selected_color), 4);
    glm::ivec2 selected_cell = board.getSelectedCell();
    getShaders()->addStaticUniform("selected_cell", glm::value_ptr(selected_cell), 2);
    getShaders()->addTexture("_texture", tex);
}
BoardOverlay::~BoardOverlay() {
    tex.reset();
    if (this->value_font)
        FT_Done_Face(this->value_font);
    if (this->mark_font)
        FT_Done_Face(this->mark_font);
    if (this->library)
        FT_Done_FreeType(this->library);
}

void BoardOverlay::reload() {
}
void BoardOverlay::handleMouseUp(const int &x, const int &y, const MouseButtonState &buttons) {
    handleMouseDown(x, y, buttons);
}
void BoardOverlay::handleMouseDrag(const int &x, const int &y, const MouseButtonState &buttons) {
    handleMouseDown(x, y, buttons);
}
void BoardOverlay::handleMouseDown(const int &x, const int &y, const MouseButtonState &buttons) {
    if (buttons.left) {
        int big_cell_width = static_cast<int>(thick_line_width + (2 * thin_line_width) + (3 * cell_width_height));
        int little_cell_width =  static_cast<int>(thin_line_width + cell_width_height);

        // Workout if we are line or background
        const glm::ivec2 big_cell_index = glm::ivec2(x, y) / big_cell_width;
        const glm::ivec2 big_cell_offset = glm::ivec2(x, y) - (big_cell_index * big_cell_width) - glm::ivec2(thick_line_width);
        const glm::ivec2 little_cell_index = big_cell_offset / little_cell_width;
        const glm::ivec2 little_cell_offset = big_cell_offset - (little_cell_index * little_cell_width);
        const glm::ivec2 cell_index = (big_cell_index * 3) + little_cell_index;

        const glm::bvec2 lb = glm::greaterThanEqual(little_cell_offset, glm::ivec2(0));
        const glm::bvec2 ub = glm::lessThan(little_cell_offset, glm::ivec2(cell_width_height));
        if ((!(lb.x && ub.x) || !(lb.y && ub.y))) {
            // We are line
            board.setSelectedCell(-1, -1);
        } else {
            // We are cell
            board.setSelectedCell(cell_index.x+1, cell_index.y+1);
        }
    }
}
void BoardOverlay::loseFocus() {
    // Set an invalid selection
    board.setSelectedCell(-1, -1);
}
void BoardOverlay::selectCell(const int &x, const int &y) {
    glm::ivec2 selected_cell = glm::ivec2(x-1, y-1);
    getShaders()->addStaticUniform("selected_cell", glm::value_ptr(selected_cell), 2);
}
void BoardOverlay::scaleBoard(const unsigned int &width_height) {
    line_width = (4 * thick_line_width) + (6 * thin_line_width);
    cell_width_height = ((width_height-line_width)/9);
    board_width_height = (cell_width_height * 9) + line_width;

    // Update shader uniforms
    int __lw = static_cast<int>(thick_line_width);
    int _lw = static_cast<int>(thin_line_width);
    int cwh = static_cast<int>(cell_width_height);
    int bd[3] = { static_cast<int>(board_width_height), static_cast<int>(board_width_height) };
    getShaders()->addStaticUniform("thick_line_width", &__lw);
    getShaders()->addStaticUniform("thin_line_width", &_lw);
    getShaders()->addStaticUniform("cell_width", &cwh);
    getShaders()->addStaticUniform("board_dims", bd, 2);

    // Resize
    setDimensions(board_width_height, board_width_height);
    tex->resize(glm::uvec2(board_width_height));

    // Resize font
    value_height = static_cast<unsigned int>(cell_width_height * 0.8);
    mark_height = static_cast<unsigned int>(cell_width_height * 0.15);
    FT_Error error = FT_Set_Pixel_Sizes(this->value_font, 0, value_height);
    if (error) {
        THROW FontLoadingError("An unexpected error occurred whilst setting big font size: %i\n", error);
    }
    error = FT_Set_Pixel_Sizes(this->mark_font, 0, mark_height);
    if (error) {
        THROW FontLoadingError("An unexpected error occurred whilst setting small font size: %i\n", error);
    }
    // Setup glyphs
    memset(value_glyph, 0, sizeof(value_glyph));
    memset(mark_glyph, 0, sizeof(mark_glyph));
    for (int i = 0; i < 9; ++i) {
        {
            value_glyph[i].c = std::to_string(i+1).c_str()[0];
            value_glyph[i].index = FT_Get_Char_Index(value_font, value_glyph[i].c);
            error = FT_Load_Glyph(value_font, value_glyph[i].index, FT_LOAD_TARGET_LIGHT|FT_LOAD_FORCE_AUTOHINT);  // FT_LOAD_DEFAULT, FT_LOAD_TARGET_LIGHT
            if (error) {
                THROW FontLoadingError("Unable to load glyph: %i\n", error);
            }
            error = FT_Get_Glyph(value_font->glyph, &value_glyph[i].image);
            if (error) {
                THROW FontLoadingError("Unable to fetch glyph: %i\n", error);
            }
            // Translate the glyph??
            FT_Glyph_Transform(value_glyph[i].image, 0, &value_glyph[i].pos);
            FT_Glyph_Get_CBox(value_glyph[i].image, ft_glyph_bbox_pixels, &value_glyph[i].bbox);
            // Convert glyph to bitmap
            error = FT_Glyph_To_Bitmap(
                &value_glyph[i].image,
                FT_RENDER_MODE_LIGHT,  // FT_RENDER_MODE_NORMAL, FT_RENDER_MODE_MONO, FT_RENDER_MODE_LIGHT
                nullptr,  // no additional translation
                1);  // destroy copy in "image"
            if (error) {
                THROW FontLoadingError("Unable to convert glyph to bitmap: %i\n", error);
            }
        }
        {
            mark_glyph[i].c = std::to_string(i+1).c_str()[0];
            mark_glyph[i].index = FT_Get_Char_Index(value_font, mark_glyph[i].c);
            error = FT_Load_Glyph(mark_font, mark_glyph[i].index, FT_LOAD_TARGET_LIGHT|FT_LOAD_FORCE_AUTOHINT);  // FT_LOAD_DEFAULT, FT_LOAD_TARGET_LIGHT
            if (error) {
                THROW FontLoadingError("Unable to load glyph: %i\n", error);
            }
            error = FT_Get_Glyph(mark_font->glyph, &mark_glyph[i].image);
            if (error) {
                THROW FontLoadingError("Unable to fetch glyph: %i\n", error);
            }
            // Translate the glyph??
            FT_Glyph_Transform(mark_glyph[i].image, 0, &mark_glyph[i].pos);
            FT_Glyph_Get_CBox(mark_glyph[i].image, ft_glyph_bbox_pixels, &mark_glyph[i].bbox);
            // Convert glyph to bitmap
            error = FT_Glyph_To_Bitmap(
                &mark_glyph[i].image,
                FT_RENDER_MODE_NORMAL,  // FT_RENDER_MODE_NORMAL, FT_RENDER_MODE_MONO, FT_RENDER_MODE_LIGHT
                nullptr,  // no additional translation
                1);  // destroy copy in "image"
            if (error) {
                THROW FontLoadingError("Unable to convert glyph to bitmap: %i\n", error);
            }
        }
    }

    queueRedrawAllCells();
}

void BoardOverlay::update() {
    bool updateRequired = false;
    {
        const std::lock_guard<std::mutex> lock(redraw_queue_mutex);
        for (auto &_c : redraw_queue) {
            const int &x = _c.first;
            const int &y = _c.second;
            // Grab cell
            Board::Cell &c = board(x, y);
            // Clear texture
            tex->clearCell(x, y);
            // Apply glyphs to cell
            const glm::ivec2 cell_begin = static_cast<int>(thin_line_width) * glm::ivec2(x, y)
                + ((glm::ivec2(x-1, y-1)/3) + glm::ivec2(1)) * glm::ivec2(thick_line_width - thin_line_width)
                + (glm::ivec2(x-1, y-1) * static_cast<int>(cell_width_height));
            if (c.value) {
                // Render value num
                {
                    const TGlyph &g = value_glyph[c.value-1];
                    const FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(g.image);
                    const int penX = cell_begin.x + cell_width_height/2 - (g.bbox.xMax - g.bbox.xMin)/2;
                    const int penY = cell_begin.y + cell_width_height/2 - (g.bbox.yMax - g.bbox.yMin)/2;
                    tex->paintGlyph(bit->bitmap, penX, penY, c.wrong);
                }
            } else {
                for (int i = 1; i <= 9; ++i) {
                    if (c.marks[i].enabled) {
                        // Render mark num
                        {
                            const TGlyph &g = mark_glyph[i-1];
                            const FT_BitmapGlyph  bit = reinterpret_cast<FT_BitmapGlyph>(g.image);
                            const glm::ivec2 mark_begin = cell_begin + glm::ivec2(4) + glm::ivec2((i-1) %3, (i-1)/3) * static_cast<int>((cell_width_height-8)/3);
                            const int penX = mark_begin.x + cell_width_height/6 - (g.bbox.xMax - g.bbox.xMin)/2;
                            const int penY = mark_begin.y + cell_width_height/6 - (g.bbox.yMax - g.bbox.yMin)/2;
                            tex->paintGlyph(bit->bitmap, penX, penY, c.marks[i].wrong);
                        }
                    }
                }
            }
        }
        if (redraw_queue.size()) {
            updateRequired = true;
            redraw_queue.clear();
        }
    }
    if (updateRequired)
        tex->updateTex();
}
void BoardOverlay::queueRedrawAllCells() {
    for (int i = 1; i <= 9; ++i)
        for (int j = 1; j <= 9; ++j)
            queueRedrawCell(i, j);
}
void BoardOverlay::queueRedrawCell(const int &x, const int &y) {
    if (x<1 || x > 9 || y< 1 || y > 9) {
        THROW OutOfBounds("Cell coordinate [%d][%d] is out of bounds, valid cell coordinates are in the range [1-9][1-9].\n", x, y);
    }
    const std::lock_guard<std::mutex> lock(redraw_queue_mutex);
    redraw_queue.emplace(std::make_pair(x, y));
}

/**
 * BoardTex methods
 */
BoardOverlay::BoardTex::BoardTex(const BoardOverlay *_parent)
    : Texture2D(glm::uvec2( 1, 1 ), { GL_RG, GL_RG, sizeof(unsigned char), GL_UNSIGNED_BYTE }, nullptr, Texture::DISABLE_MIPMAP | Texture::WRAP_REPEAT)
    , texture(nullptr)
    , dimensions(1, 1)
    , parent(_parent) {
}
void BoardOverlay::BoardTex::resize(const glm::uvec2 &_dimensions) {
    this->dimensions = _dimensions;
    if (texture)
        free(texture);
    texture = reinterpret_cast<unsigned char**>(malloc(sizeof(char*) * this->dimensions.y));
    texture[0] = reinterpret_cast<unsigned char*>(malloc(sizeof(char) * this->dimensions.x * this->dimensions.y * 2));  // 2 channel
    memset(texture[0], 0, sizeof(char)*this->dimensions.x*this->dimensions.y * 2);
    for (unsigned int i = 1; i < this->dimensions.y; i++) {
        texture[i] = texture[i - 1] + (this->dimensions.x * 2);
    }
}
void BoardOverlay::BoardTex::updateTex() {
    if (!texture)
        return;
    Texture2D::resize(dimensions, texture[0]);
}
BoardOverlay::BoardTex::~BoardTex() {
    if (texture) {
        free(texture[0]);
        free(texture);
    }
}
void BoardOverlay::BoardTex::paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY, bool isRed) {
    for (unsigned int y = 0; y < glyph.rows; y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + (penX * 2);
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            if (isRed) {
                dst_ptr[x*2] = src_ptr[x];
                dst_ptr[(x*2)+1] = 0xff;
            } else {
                dst_ptr[x*2] = src_ptr[x];
            }
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}
void BoardOverlay::BoardTex::paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY, bool isRed) {
    for (unsigned int y = 0; y < glyph.rows; y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + (penX * 2);
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            for (int j = 0; j < 8; j++)
                if (((src_ptr[x] >> (7 - j)) & 1) == 1) {
                    dst_ptr[(x * 8 + j) *2] = 0xff;  // src_ptr[x];
                if (isRed)
                    dst_ptr[((x * 8 + j) *2)+1] = 0xff;
                }
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}
void BoardOverlay::BoardTex::clearCell(const int &x, const int &y) {
    if (x<1 || x > 9 || y< 1 || y > 9) {
        THROW OutOfBounds("Cell coordinate [%d][%d] is out of bounds, valid cell coordinates are in the range [1-9][1-9].\n", x, y);
    }
    const glm::ivec2 cell_begin = static_cast<int>(parent->thin_line_width) * glm::ivec2(x, y)
        + ((glm::ivec2(x-1, y-1)/3) + glm::ivec2(1)) * glm::ivec2(parent->thick_line_width - parent->thin_line_width)
        + (glm::ivec2(x-1, y-1) * static_cast<int>(parent->cell_width_height));
    const glm::ivec2 cell_end = cell_begin + glm::ivec2(parent->cell_width_height);
    for (int cell_y = cell_begin.y; cell_y < cell_end.y; ++cell_y) {
        memset(&texture[cell_y][cell_begin.x * 2], 0, (cell_end.x - cell_begin.x) * sizeof(unsigned char) * 2);
    }
}
