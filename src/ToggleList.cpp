#include "ToggleList.h"

#include <freetype/ftglyph.h>
#include <glm/gtc/type_ptr.hpp>


#include "util/fonts.h"
#include "shader/Shaders.h"

ToggleList::ToggleList(std::vector<std::string> _items, const unsigned int _fontHeight)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::TEXT))
    , fontHeight(_fontHeight)
    , lineSpacing(-0.1f)
    , padding(5)
    , tex(std::make_shared<TextureString>())
    , color(glm::vec4(0, 0, 0, 1))
    , backgroundColor(glm::vec4(0.8f, 0.8f, 0.8f, 1)) {
    // This must be clickable
    setClickable(true);
    initFont(_fontHeight);
    CHECK_OFF = {};
    CHECK_ON = {};
    for (auto i : _items) {
        ToggleItem t = {};
        t.name = i;
        items.push_back(t);
    }
    redrawList();
    getShaders()->addStaticUniform("_col", glm::value_ptr(this->color), 4);
    getShaders()->addStaticUniform("_backCol", glm::value_ptr(this->backgroundColor), 4);
    getShaders()->addTexture("_texture", tex);
}
ToggleList::~ToggleList() {
    FT_Done_Glyph(CHECK_OFF.image);
    FT_Done_Glyph(CHECK_ON.image);
}

void ToggleList::initFont(const unsigned int &font_height) {
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "An unexpected error occurred whilst initialising FreeType: %i\n", error);
        return;
    }
    error = FT_New_Face(library,
        fonts::findFont({"Segoe UI Symbol"}, fonts::GenericFontFamily::SANS).c_str(),
        0,
        &font);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "Unable to load 'Segoe UI Symbol' font for ToggleList, checkboxes may not work.\n");
        error = FT_New_Face(library,
            fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str(),
            0,
            &this->font);
    }
    if (error) {
        fprintf(stderr, "An unexpected error occurred whilst loading font file: %i\n", error);
    }
    // Switch to unicode charmap
    error =  FT_Select_Charmap(this->font, ft_encoding_unicode);  // ft_encoding_symbol
    if (error) {
        fprintf(stderr, "An unexpected error occurred whilst switching to unicode charmap: %i\n", error);
    }
    error = FT_Set_Pixel_Sizes(this->font, 0, font_height);
    if (error) {
        fprintf(stderr, "An unexpected error occurred whilst setting font size: %i\n", error);
    }
}

bool ToggleList::getState(const std::string &name) {
    for (auto &i : items) {
        if (i.name == name) {
            return i.state;
        }
    }
    THROW OutOfBounds("Item with name '%s' not found in toggle list.", name.c_str());
}
void ToggleList::setState(const std::string &name, const bool &state) {
    for (auto &i : items) {
        if (i.name == name) {
            i.state = state;
            redrawCheck(i);
            return;
        }
    }
    THROW OutOfBounds("Item with name '%s' not found in toggle list.", name.c_str());
}
ToggleList::TGlyph2 ToggleList::loadGlyph(wchar_t c, int &penX, int &penY, const TGlyph2 &previous) {
    TGlyph2 glyph;
    glyph.c = c;
    glyph.index = FT_Get_Char_Index(font, glyph.c);
    if (glyph.index == 0) {
        printf("No glyph found for char '%wc'\n", glyph.c);
    }
    // Add kerning if present
    if (FT_HAS_KERNING(font) && previous.image && glyph.index) {
        FT_Vector  delta;
        FT_Get_Kerning(font, previous.index, glyph.index, FT_KERNING_DEFAULT, &delta);
        penX += delta.x >> 6;
    }
    glyph.pos.x = penX * 64;
    glyph.pos.y = penY * 64;

    FT_Error error = FT_Load_Glyph(font, glyph.index, FT_LOAD_TARGET_LIGHT|FT_LOAD_FORCE_AUTOHINT);  // FT_LOAD_DEFAULT, FT_LOAD_TARGET_LIGHT
    if (error) return glyph;

    error = FT_Get_Glyph(font->glyph, &glyph.image);
    if (error) return glyph;
    // Translate the glyph image
    FT_Glyph_Transform(glyph.image, nullptr, &glyph.pos);

    // Move along pen
    penX += font->glyph->advance.x >> 6;

    return glyph;
}
void ToggleList::redrawList() {
    if (items.empty()) return;
    FT_Error error;
    // Prep long lived check bitmaps
    int penX = 0, penY = 0;
    CHECK_ON = loadGlyph(L'☑', penX, penY);
    error = FT_Glyph_To_Bitmap(&CHECK_ON.image, FT_RENDER_MODE_LIGHT, nullptr, 1);
    visassert(!error);
    penX = 0, penY = 0;
    CHECK_OFF = loadGlyph(L'☐', penX, penY);
    error = FT_Glyph_To_Bitmap(&CHECK_OFF.image, FT_RENDER_MODE_LIGHT, nullptr, 1);
    visassert(!error);
    // For each item
    penX = 0, penY = 0;
    FT_BBox  bbox;
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;
    const int lineHeight = static_cast<int>((this->font->size->metrics.height >> 6));
    const int ascender = this->font->size->metrics.ascender >> 6;
    const int descender = this->font->size->metrics.descender >> 6;
    std::vector<std::vector<TGlyph2>> item_glyphs;
    for (int line = 0; line < static_cast<int>(items.size()); ++line) {
        auto &item = items[line];
        const size_t string_length = strlen(item.name.c_str()) + 2;
        // First load and position all glyphs on a straight line
        std::vector<TGlyph2> glyphs;
        penX = 0;
        // Load char 1 (checkbox)
        glyphs.push_back(loadGlyph(L'☑', penX, penY));  // Placeholder char
        // Load char 2 (space)
        glyphs.push_back(loadGlyph(' ', penX, penY, *(--glyphs.end())));
        // Rest of string
        for (unsigned int n = 0; n < string_length - 2; n++) {
            glyphs.push_back(loadGlyph(item.name.c_str()[n], penX, penY, *(--glyphs.end())));
        }
        // Calculate bounding box
        for (auto &g : glyphs) {
            FT_BBox  glyph_bbox;
            FT_Glyph_Get_CBox(g.image, ft_glyph_bbox_pixels, &glyph_bbox);
            item.lineMin = std::min<int>((line * lineHeight * (lineSpacing + 1.0f)) + glyph_bbox.yMin + padding - descender, item.lineMin);
            item.lineMax = std::max<int>((line * lineHeight * (lineSpacing + 1.0f)) + glyph_bbox.yMax + padding - descender, item.lineMax);
            glyph_bbox.yMin = static_cast<FT_Pos>(glyph_bbox.yMax > ascender ? (line * lineHeight * (lineSpacing + 1.0f)) - (static_cast<int>(glyph_bbox.yMax) - ascender) : (line * lineHeight * (lineSpacing + 1.0f)));
            glyph_bbox.yMax = static_cast<FT_Pos>(glyph_bbox.yMin < descender ? (line + 1) * lineHeight * (lineSpacing + 1.0f) + (descender - glyph_bbox.yMin) : ((line + 1) * lineHeight * (lineSpacing + 1.0f)));
            bbox.xMin = std::min(glyph_bbox.xMin, bbox.xMin);
            bbox.yMin = std::min(glyph_bbox.yMin, bbox.yMin);
            bbox.xMax = std::max(glyph_bbox.xMax, bbox.xMax);
            bbox.yMax = std::max(glyph_bbox.yMax, bbox.yMax);
        }
        item_glyphs.push_back(glyphs);
    }
    // And thus the texture size
     glm::uvec2 texDim(
    (2 * padding) + bbox.xMax - bbox.xMin,
    (2 * padding) + bbox.yMax - bbox.yMin - lineHeight * lineSpacing);
    // Iterate chars, painting them to tex
    tex->resize(texDim);
    for (int line = 0; line < static_cast<int>(item_glyphs.size()); ++line) {
        auto &line_glyphs = item_glyphs[line];
        for (int item = 0; item < line_glyphs.size(); ++item) {
            auto &glyph = line_glyphs[item];
            if (!(glyph.c == '\n' || glyph.c == '\r')) {
                error = FT_Glyph_To_Bitmap(
                    &glyph.image,
                    FT_RENDER_MODE_LIGHT,  // FT_RENDER_MODE_NORMAL, FT_RENDER_MODE_MONO, FT_RENDER_MODE_LIGHT
                    nullptr,  // no additional translation
                    1);  // destroy copy in "image"
                if (!error) {
                    FT_BitmapGlyph bit = reinterpret_cast<FT_BitmapGlyph>(glyph.image);
                    penX = static_cast<int>(padding) + bit->left - bbox.xMin;
                    penY = static_cast<int>(padding) - bit->top + ascender + static_cast<int>(lineHeight * line * (lineSpacing + 1.0f));
                    // Only paint is the glyph is within bounds of the texture (report err if our maths is bad)
                    if (penX >= 0 && penX + bit->bitmap.pitch <= static_cast<int>(texDim.x) && penY >= 0 && penY + static_cast<int>(bit->bitmap.rows) <= static_cast<int>(texDim.y)) {
                        if (item == 0) {
                            // First char is always check symbol
                            FT_BitmapGlyph check_bit = reinterpret_cast<FT_BitmapGlyph>(items[line].state ? CHECK_ON.image : CHECK_OFF.image);
                            items[line].penX = penX;
                            items[line].penY = penY;
                            tex->paintGlyph(check_bit->bitmap, penX, penY);
                        } else {
                            tex->paintGlyph(bit->bitmap, penX, penY);
                        }
                    } else {
                        fprintf(stderr, "Skipped painting char '%wc' of '%s' to avoid writing out of bounds.%i\n", glyph.c, items[line].name.c_str(), static_cast<int>(FT_IS_SCALABLE(this->font)));
                    }
                }
            }
            FT_Done_Glyph(glyph.image);
        }
    }
    // link tex to shader
    tex->updateTex();
    // Set width
    setDimensions(texDim);
}
void ToggleList::reload() {
    redrawList();
}

void ToggleList::redrawCheck(const ToggleItem &ti) {
    FT_BitmapGlyph check_bit = reinterpret_cast<FT_BitmapGlyph>(ti.state ? CHECK_ON.image : CHECK_OFF.image);
    tex->paintGlyph(check_bit->bitmap, ti.penX, ti.penY);
    tex->updateTex();
}


void ToggleList::handleMouseDown(const int &x, const int &y, const MouseButtonState &buttons) {
    const int inverted_y = getHeight() - y;
    if (buttons.left) {
        for (auto &item : items) {
            // Calculate which item it intersects
            if (inverted_y <= item.lineMax && inverted_y >= item.lineMin) {
                // Toggle item state
                item.state = !item.state;
                // Send item to redraw
                redrawCheck(item);
            }
        }
    }
}

ToggleList::TextureString::TextureString()
    : Texture2D(glm::uvec2( 1, 1 ), { GL_RED, GL_RED, sizeof(unsigned char), GL_UNSIGNED_BYTE }, nullptr, Texture::DISABLE_MIPMAP | Texture::WRAP_REPEAT)
    , texture(nullptr)
    , dimensions(1, 1) {
}
void ToggleList::TextureString::resize(const glm::uvec2 &_dimensions) {
    this->dimensions = _dimensions;
    if (texture)
        free(texture);
    texture = reinterpret_cast<unsigned char**>(malloc(sizeof(char*) * this->dimensions.y));
    texture[0] = reinterpret_cast<unsigned char*>(malloc(sizeof(char) * this->dimensions.x * this->dimensions.y));
    memset(texture[0], 0, sizeof(char)*this->dimensions.x*this->dimensions.y);
    for (unsigned int i = 1; i < this->dimensions.y; i++) {
        texture[i] = texture[i - 1] + this->dimensions.x;
    }
}
void ToggleList::TextureString::updateTex() {
    if (!texture)
        return;
    Texture2D::resize(dimensions, texture[0]);
}
ToggleList::TextureString::~TextureString() {
    if (texture) {
        free(texture[0]);
        free(texture);
    }
}
void ToggleList::TextureString::paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY) {
    for (unsigned int y = 0; y < glyph.rows; y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + penX;
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            dst_ptr[x] = src_ptr[x];
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}
void ToggleList::TextureString::paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY) {
    for (unsigned int y = 0; y < glyph.rows; y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + penX;
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            for (int j = 0; j < 8; j++)
                if (((src_ptr[x] >> (7 - j)) & 1) == 1)
                    dst_ptr[x * 8 + j] = 0xff;  // src_ptr[x];
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}
