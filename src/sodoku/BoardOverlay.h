#ifndef SRC_SODOKU_BOARDOVERLAY_H_
#define SRC_SODOKU_BOARDOVERLAY_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftglyph.h>

#include <memory>
#include <string>

#include "Overlay.h"

class BoardOverlay : public Overlay {
    friend class BoardTex;
    class BoardTex : public Texture2D {
     public:
        /**
         * Creates a new TextureString which represents the texture holding the glyphs of the string
         */
        explicit BoardTex(const BoardOverlay *parent);
        /**
         * Frees the texture's data
         */
        ~BoardTex();
        /**
         * Resizes the texture
         * @param _dimensions The dimensions of the texture to be created
         */
        void resize(const glm::uvec2 &_dimensions) override;
        using RenderTarget::resize;
        /**
         * Paints a single character glyph to the texture at the specified location from a 1-byte texture
         * @param penX The x coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         * @param penY The y coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         */
        void paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
        /**
         * Paints a single character glyph to the texture at the specified location from a 1-bit mono texture
         * @param penX The x coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         * @param penY The y coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         */
        void paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
        /**
         * Updates the GL texture to match the painted texture
         */
        void updateTex();

        void clearCell(const int &x, const int &y);

     private:
        unsigned char **texture;
        glm::uvec2 dimensions;
        const BoardOverlay *parent;
    };
    struct  TGlyph {
        FT_UInt    index;  /* glyph index                  */
        FT_Vector  pos;    /* glyph origin on the baseline */
        FT_Glyph   image;  /* glyph image                  */
        char       c;      /* char                         */
        FT_BBox    bbox;   /* Bounding box                 */
    };

 public:
    explicit BoardOverlay(const unsigned int &width_height = 720);
    ~BoardOverlay();
    void reload() override;

    void handleMouseUp(const int &x, const int &y, const MouseButtonState &buttons) override;
    void handleMouseDown(const int &x, const int &y, const MouseButtonState &buttons) override;
    void handleMouseDrag(const int &x, const int &y, const MouseButtonState &buttons) override;
    void loseFocus() override;
    void selectCell(const int &x, const int &y);
    /**
     * Redraw the texture of the specified cell
     * This resets the texture and replaces the glypths
     * @param x Cell x coord (1-indexed)
     * @param y Cell y coord (1-indexed)
     */
    void redrawCell(const int &x, const int &y);
    /**
     * Triggers redrawCell(int, int) for every cell
     */
    void redrawAllCells();

 private:
    void scaleBoard(const unsigned int &width_height);
    const glm::vec4 color = glm::vec4(1.0f);
    const glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 selected_color = glm::vec4(255/255.0f, 251/255.0f, 145/255.0f, 1.0f);
    // const glm::vec4 selected_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Harsher colour, can't see light yellow with nightmode enabled
    unsigned int thick_line_width = 5;

    unsigned int thin_line_width = 2;

    unsigned int board_width_height;
    glm::ivec2 selected_cell;

    // These variables are internal cache of maths for calculating grid coords
    unsigned int line_width;
    unsigned int cell_width_height;

    // Glyph rendering data
    FT_Library library;
    FT_Face value_font, mark_font;
    TGlyph value_glyph[9], mark_glyph[9];
    unsigned int value_height, mark_height;
    std::shared_ptr<BoardTex> tex;
};


#endif  // SRC_SODOKU_BOARDOVERLAY_H_
