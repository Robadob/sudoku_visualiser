#include "BoardOverlay.h"

#include <glm/gtc/type_ptr.hpp>

#include "shader/Shaders.h"

BoardOverlay::BoardOverlay(const unsigned int &width_height)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SODOKU_BOARD))
    , selected_cell(-1, -1) {
    calculateBoardLocations(width_height);
    getShaders()->addStaticUniform("_col", glm::value_ptr(this->color), 4);
    getShaders()->addStaticUniform("_backCol", glm::value_ptr(this->background_color), 4);
    getShaders()->addStaticUniform("_selCol", glm::value_ptr(this->selected_color), 4);
    getShaders()->addStaticUniform("selected_cell", glm::value_ptr(this->selected_cell), 2);
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
        if ((!(lb.x && ub.x ) || !(lb.y && ub.y))) {
            // We are line
            selectCell(-1, -1);
        } else {
            // We are cell
            selectCell(cell_index.x+1, cell_index.y+1);
        }
    }
}
void BoardOverlay::loseFocus() {
    // Set an invalid selection
    selectCell(-1, -1);
}
void BoardOverlay::selectCell(const int &x, const int &y) {
    printf( "Select Cell: (%d, %d)\n",x, y);
    selected_cell = glm::ivec2(x-1, y-1);
    getShaders()->addStaticUniform("selected_cell", glm::value_ptr(this->selected_cell), 2);
}
void BoardOverlay::calculateBoardLocations(const unsigned int &width_height) {
    line_width = (4 * thick_line_width) + (6 * thin_line_width);
    cell_width_height = ((width_height-line_width)/9);
    board_width_height = (cell_width_height * 9) + line_width;
    setDimensions(board_width_height, board_width_height);

    int __lw = static_cast<int>(thick_line_width);
    int _lw = static_cast<int>(thin_line_width);
    int cwh = static_cast<int>(cell_width_height);
    int bd[3] = { static_cast<int>(board_width_height), static_cast<int>(board_width_height) };
    getShaders()->addStaticUniform("thick_line_width", &__lw);
    getShaders()->addStaticUniform("thin_line_width", &_lw);
    getShaders()->addStaticUniform("cell_width", &cwh);
    getShaders()->addStaticUniform("board_dims", bd, 2);
   
}
