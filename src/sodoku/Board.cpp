#include "Board.h"

#include "util/VisException.h"

Board::Board()
    : selected_cell(-1) { }

Board::CellRow &Board::operator[](const int &x) {
    if (x < 1 || x > 9) {
        THROW OutOfBounds("Cell x-index [%d] is out of bounds, valid indexes are in the range [1-9].\n", x);
    }
    return cell_rows[x-1];
}

std::shared_ptr<BoardOverlay> Board::getOverlay(const unsigned int &dims) {
    if (!overlay)
        overlay = std::make_shared<BoardOverlay>(*this, dims);
    return overlay;
}
void Board::setSelectedCell(const int &x, const int &y) {
    selected_cell = glm::ivec2(x, y);
    overlay->selectCell(x, y);
}
const glm::ivec2 &Board::getSelectedCell() {
    return selected_cell;
}

void Board::handleNumberPress(const int &number, bool shift, bool ctrl, bool alt) {
    // If we have a selection
    if (selected_cell.x > 0 && selected_cell.x <= 9 &&
        selected_cell.y > 0 && selected_cell.y <= 9) {
        Cell &c = (*this)[selected_cell.x][selected_cell.y];
        if (shift) {
            // Ensure main value is disabled (don't set 0, that nukes marks)
            c.flags.enabled = false;
            // Toggle the user's mark
            c.marks[number].enabled = !c.marks[number].enabled;
        } else {
            // Change value to number
            // This resets if the number is 0
            c = number;
        }
        // Tell cell to redraw
        overlay->redrawCell(selected_cell.x, selected_cell.y);
    }
}

/**
 * Nested util class methods
 */
Board::Cell::Flags &Board::Cell::Marks::operator[](const int &i) {
    if (i < 1 || i > 9) {
        THROW OutOfBounds("Mark index [%d] is out of bounds, valid indexes are in the range [1-9].\n", i);
    }
    return flags[i-1];
}

Board::Cell::Cell()
    : value(0)
    , flags({})
    , marks({}) { }

Board::Cell &Board::Cell::operator=(const int &i) {
    if (i < 0 || i > 9) {
        THROW OutOfBounds("Value of %d is out of bounds, valid indexes are in the range [0-9].\n", i);
    }
    if (i == 0) {
        // Reset square
        flags.enabled = false;
        flags.wrong = false;
    } else {
        // Enable var
        flags.enabled = true;
    }
    value = i;
    // Kill all marks
    for (int f = 1; f <= 9; ++f) {
        marks[f].enabled = false;
    }
    return *this;
}
Board::Cell &Board::CellRow::operator[](const int &y) {
    if (y < 1 || y > 9) {
        THROW OutOfBounds("Cell y-index [%d] is out of bounds, valid indexes are in the range [1-9].\n", y);
    }
    return cols[y-1];
}
