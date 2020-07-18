#ifndef SRC_SODOKU_BOARDOVERLAY_H_
#define SRC_SODOKU_BOARDOVERLAY_H_
#include "Overlay.h"

class BoardOverlay : public Overlay {

public:
    BoardOverlay(const unsigned int &width_height = 720);
    void reload() override;
    
    void handleMouseUp(const int &x, const int &y, const MouseButtonState &buttons) override;
    void handleMouseDown(const int &x, const int &y, const MouseButtonState &buttons) override;
    void handleMouseDrag(const int &x, const int &y, const MouseButtonState &buttons) override;
    void loseFocus() override;
    void selectCell(const int &x, const int &y);
private:
    void calculateBoardLocations(const unsigned int &width_height);
    const glm::vec4 color = glm::vec4(1.0f);
    const glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 selected_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // glm::vec4(255/255.0f, 251/255.0f, 145/255.0f, 1.0f);  // Harsher colour, can't see light yellow with nightmode enabled
    unsigned int thick_line_width = 5;

    unsigned int thin_line_width = 2;

    unsigned int board_width_height;
    glm::ivec2 selected_cell;

    // These variables are internal cache of maths for calculating grid coords
    unsigned int line_width;
    unsigned int cell_width_height;
};


#endif  // SRC_SODOKU_BOARDOVERLAY_H_
