#include "Board.h"

#include <SDL_keycode.h>


#include "ConstraintHints.h"
#include "ConstraintValidator.h"
#include "util/VisException.h"

Board::Board() { }

Board::Cell &Board::operator()(const int &x, const int &y) {
    if (x < 1 || x > 9) {
        THROW OutOfBounds("Cell x-index [%d] is out of bounds, valid indexes are in the range [1-9].\n", x);
    }
    if (y < 1 || y > 9) {
        THROW OutOfBounds("Cell y-index [%d] is out of bounds, valid indexes are in the range [1-9].\n", y);
    }
    return transposeState ? raw_board[y-1][x-1] : raw_board[x-1][y-1];
}
Board::Cell &Board::operator()(const Pos &xy) {
    return (*this)(xy.x, xy.y);
}

std::shared_ptr<BoardOverlay> Board::getOverlay(const unsigned int &dims) {
    if (!overlay)
        overlay = std::make_shared<BoardOverlay>(*this, dims);
    return overlay;
}
void Board::killOverlay() {
    overlay = nullptr;
}
bool Board::hasOverlay() const {
    return overlay != nullptr;
}
void Board::setSelectedCell(const int &x, const int &y) {
    selected_cell = glm::ivec2(x, y);
    if (overlay)
        overlay->selectCell(x, y);
}
void Board::shiftSelectedCell(const int &x, const int &y) {
    selected_cell += glm::ivec2(x, y);
    selected_cell = glm::clamp(static_cast<glm::ivec2>(selected_cell), glm::ivec2(1), glm::ivec2(9));
    if (overlay)
        overlay->selectCell(selected_cell.x, selected_cell.y);
}
const Board::Pos &Board::getSelectedCell() {
    return selected_cell;
}
void Board::setMode(const Mode &mode) {
    current_mode = mode;
    validate();
}

void Board::handleKeyPress(const SDL_Keycode &keycode, bool shift, bool ctrl, bool alt) {
    if (keycode == SDLK_z && ctrl && !shift) {
        // Undo
        if (!undoStack.empty()) {
            RawBoard undo = undoStack.top();
            undoStack.pop();
            // Swap the cell off stack with cell on board
            std::swap(undo, raw_board);
            // Move the UndoPair into the redo stack
            redoStack.push(undo);
            // Tell to validate, this forces redraw all
            validate();
        }
    } else if (keycode == SDLK_y && ctrl && !shift) {
        // Redo
        if (!redoStack.empty()) {
            RawBoard redo = redoStack.top();
            redoStack.pop();
            // Swap the cell off stack with cell on board
            std::swap(redo, raw_board);
            // Move the UndoPair into the redo stack
            undoStack.push(redo);
            // Tell to validate, this forces redraw all
            validate();
        }
    } else if (keycode == SDLK_h && !ctrl && !shift) {
        hint();
    } else if (keycode == SDLK_c && !ctrl && !shift) {
        clear();
    } else {
        // If we have a selection
        if (selected_cell.x > 0 && selected_cell.x <= 9 &&
            selected_cell.y > 0 && selected_cell.y <= 9) {
            int number = keycode == SDLK_BACKSPACE ? 0 : keycode- SDLK_0;
            if (number < 0 || number > 9)
                return;
            // Clear redo stack as soon as user makes a change
            while (!redoStack.empty()) { redoStack.pop(); }
            Cell &c = (*this)(selected_cell.x, selected_cell.y);
            // Add the selected cell to the undo stack
            undoStack.push(raw_board);
            if (shift && !ctrl) {
                // Ensure main value is disabled
                c = 0;
                if (number) {
                    // Toggle the user's mark
                    c.marks[number].enabled = !c.marks[number].enabled;
                    // Clear wrong flag
                    c.marks[number].wrong = false;
                } else {
                    // User pressed shift + 0, set all marks
                    // (They can just press 0 to clear all marks)
                    for (int i = 1; i <= 9; ++i) {
                        c.marks[i].enabled = true;
                    }
                }
            } else if (ctrl && !shift) {
                // Ensure main value is disabled
                c = 0;
                if (number) {
                    // Set the mark enabled
                    c.marks[number].enabled = true;
                    // Flag the mark to be painted red
                    c.marks[number].wrong = !c.marks[number].wrong;
                } else {
                    // User pressed ctrl + 0
                    // If they have any marks enabled + wrong, disable all, else enable all
                    bool has_wrong = false;
                    for (int i = 1; i <= 9; ++i) {
                        if (c.marks[i].enabled && c.marks[i].wrong) {
                            has_wrong = true;
                            break;
                        }
                    }
                    if (has_wrong) {
                        for (int i = 1; i <= 9; ++i) {
                            c.marks[i].wrong = false;
                        }
                    } else {
                        for (int i = 1; i <= 9; ++i) {
                            if (c.marks[i].enabled)
                                c.marks[i].wrong = true;
                        }
                    }
                }
            } else {
                if (c != number) {
                    // Change value to number
                    // This disables the cell if the number is 0
                    c = number;
                    c.clearMarks();
                } else {
                    // User action has no effect, so remove it off the undo stack
                    undoStack.pop();
                    return;
                }
            }
            // Tell to validate, this forces redraw all
            validate();
        }
    }
}
bool Board::validate() {
    clearWrong();
    if (current_mode == None) {
        lastValidateResult =  true;
    } else if (current_mode == Vanilla) {
        lastValidateResult = ConstraintValidator::vanilla(*this);
    } else {
        THROW ValidationError("Unexpected Mode\n");
    }
    if (overlay)
        overlay->queueRedrawAllCells();
    return lastValidateResult;
}
void Board::hint() {
    // Cannot provide a hint, if board contains errors
    if (lastValidateResult) {
        // Add the selected cell to the undo stack
        undoStack.push(raw_board);
        // Enable all marks
        // We do this first, so that subsequent method calls can mark wrong any which are not possible
        for (int x = 1; x <= 9; ++x) {
            for (int y = 1; y <= 9; ++y) {
                (*this)(x, y).setMarks();
            }
        }
        // Call corresponding hint method
        if (current_mode == Vanilla) {
            ConstraintHints::vanilla(*this);
        } else {
            // We didn't do anything, so undo the mark changes and return
            raw_board = undoStack.top();
            undoStack.pop();
            return;
        }
        if (overlay)
            overlay->queueRedrawAllCells();
    }
}
void Board::clear() {
    for (int x = 1; x <= 9; ++x) {
        for (int y = 1; y <= 9; ++y) {
            (*this)(x, y) = 0;
            (*this)(x, y).clearMarks();
        }
    }
    if (overlay)
        overlay->queueRedrawAllCells();
}
void Board::clearWrong() {
    for (int x = 1; x<= 9; ++x) {
        for (int y = 1; y<= 9; ++y) {
            (*this)(x, y).wrong = false;
        }
    }
}

/**
 * Nested util class methods
 */
Board::Cell::Marks::Flags &Board::Cell::Marks::operator[](const int &i) {
    if (i < 1 || i > 9) {
        THROW OutOfBounds("Mark index [%d] is out of bounds, valid indexes are in the range [1-9].\n", i);
    }
    return flags[i-1];
}

Board::Cell::Cell()
    : value(0)
    , wrong(false)
    , marks({}) { }

bool Board::Cell::operator==(const int &other) const {
    return this->value == other && this->value != 0;
}
bool Board::Cell::operator==(const Cell &other) const {
    if (this->value == 0 && other.value == 0) {
        if (this->marks != other.marks)
            return false;
    } else if (this->value != other.value) {
        return false;
    }
    return true;
}
bool Board::Cell::operator!=(const int &other) const {
    return !(*this == other);
}
bool Board::Cell::operator!=(const Cell &other) const {
    return  !(*this == other);
}
Board::Cell &Board::Cell::operator=(const unsigned int &i) {
    if (i < 0 || i > 9) {
        THROW OutOfBounds("Value of %d is out of bounds, valid indexes are in the range [0-9].\n", i);
    }
    value = i;
    wrong = false;
    return *this;
}
void Board::Cell::clearMarks() {
    // Kill all marks
    for (int f = 1; f <= 9; ++f) {
        marks[f].enabled = false;
        marks[f].wrong = false;
    }
}
void Board::Cell::setMarks() {
    if (!value) {
        for (int f = 1; f <= 9; ++f) {
            marks[f].enabled = true;
            marks[f].wrong = false;
        }
    }
}
unsigned char Board::Cell::rawValue() {
    if (value) {
        return value;
    }
    unsigned int val = 0;
    for (int f = 1; f <= 9; ++f) {
        if (marks[f].enabled && !marks[f].wrong) {
            if (!val)
                val = f;
            else
                return 0;
        }
    }
    return val;
}

bool Board::Cell::Marks::operator==(const Marks &other) const {
    for (int f = 0; f < 9; ++f) {
        if (flags[f].enabled != other.flags[f].enabled || flags[f].wrong != other.flags[f].wrong)
            return false;
    }
    return true;
}
bool Board::Cell::Marks::operator!=(const Marks &other) const {
    return !(*this == other);
}
