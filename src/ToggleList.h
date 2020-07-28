#ifndef SRC_TOGGLELIST_H_
#define SRC_TOGGLELIST_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "Overlay.h"
#include "freetype/ftglyph.h"

/**
 * A HUD object for providing clickable on:off items
 * The status is represented by a unicode checkbox
 * This currently relies on the font Segoe UI Symbol
 */
class ToggleList : public Overlay {
    struct ToggleItem {
        ToggleItem(const std::string &_name, std::reference_wrapper<bool> _state)
            : name(_name)
            , state(_state) { }
        std::string name;
        std::reference_wrapper<bool> state;
        unsigned int index;
        unsigned int y_offset = 0;
        int penX = 0;
        int penY = 0;
        int lineMin = INT_MAX;
        int lineMax = 0;
    };
    class TextureString : public Texture2D {
     public:
        /**
         * Creates a new TextureString which represents the texture holding the glyphs of the string
         */
        TextureString();
        /**
         * Frees the texture's data
         */
        ~TextureString();
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

     private:
        unsigned char **texture;
        glm::uvec2 dimensions;
    };

 public:
    /**
     * @param items Vector of list items, a copy of this will be taken.
     * @param _fontHeight Height of font in pixels
     */
    explicit ToggleList(std::vector<std::pair<std::string, std::reference_wrapper<bool>>> items, const unsigned int _fontHeight = 25);
    ~ToggleList();
    bool getState(const std::string &name);
    void setState(const std::string &name, const bool &state);
    void handleMouseDown(const int &x, const int &y, const MouseButtonState &buttons) override;
    void reload() override;

 private:
    void redrawList();
struct TGlyph2 {
    FT_UInt    index = 0;  /* glyph index                  */
    FT_Vector  pos = {0, 0};    /* glyph origin on the baseline */
    FT_Glyph   image = nullptr;  /* glyph image                  */
    wchar_t       c = 0;      /* char                         */
};
    TGlyph2 loadGlyph(wchar_t c, int &penX, int &penY, const TGlyph2 &previous = {});
    void redrawCheck(const ToggleItem &ti);
    /**
     * Ordered list of item names
     */
    std::vector<ToggleItem> items;
    /**
     * Font stuff
     */    
    FT_Library  library;
    FT_Face     font;
    unsigned int fontHeight;
    float lineSpacing;
    unsigned int padding;
    void initFont(const unsigned int &font_height);
    TGlyph2 CHECK_ON, CHECK_OFF;
    std::shared_ptr<TextureString> tex;
    glm::vec4 color;
    glm::vec4 backgroundColor;
};

#endif  // SRC_TOGGLELIST_H_
