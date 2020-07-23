#ifndef SRC_SUDOKU_BOARD_H_
#define SRC_SUDOKU_BOARD_H_

#include <memory>
#include <string>
#include <array>
#include <stack>
#include <utility>

#include "sudoku/BoardOverlay.h"

/**
 * Which collection of validation rules to apply
 * @note In future this could be converted to a bitmask for individual rules
 */
enum Mode : unsigned char {
    None = 0,
    Vanilla,
    End  // End is not a valid mode, it acts as the end point, so we can iterate the enum
};
inline std::string to_string(const Mode&m) {
    switch (m) {
        case None: return "No Constraints";
        case Vanilla: return "Vanilla Constraints";
        default: return "Invalid";
    }
}
class Board {
 public:
    /**
     * Represents a location on the board
     */
    struct Pos {
        Pos() : x(0), y(0) { }
        Pos(const int &_x, const int &_y) : x(_x), y(_y) { }  // Should probably do some validation to check in range
        explicit Pos(const glm::ivec2&xy) : x(xy.x), y(xy.y) { }  // Should probably do some validation to check in range
        unsigned char x : 4, y: 4;
        /**
         * Returns true if the position is valid
         */
        explicit operator bool() const {
            return x >= 1 && x <= 9 && y >= 1 && y <= 9;
        }
        operator glm::ivec2() const {
            return glm::ivec2(x, y);
        }
        Pos &operator=(const glm::ivec2 &xy) {
            x = xy.x;
            y = xy.y;
            return *this;
        }
        Pos &operator+=(const Pos &xy) {
            x += xy.x;
            y += xy.y;
            return *this;
        }
        Pos &operator+=(const glm::ivec2 &xy) {
            x += xy.x;
            y += xy.y;
            return *this;
        }
        Pos &operator-=(const Pos &xy) {
            x -= xy.x;
            y -= xy.y;
            return *this;
        }
        Pos &operator-=(const glm::ivec2 &xy) {
            x -= xy.x;
            y -= xy.y;
            return *this;
        }
    };
    /**
     * Represents a single number that can be written into a sudoku board
     */
    struct Cell {
        /**
         * The mini numbers within a cell
         * Only visible when the value for the cell is not set
         */
        struct Marks {
            struct Flags {
                unsigned char enabled:1;
                unsigned char wrong:1;
            };
            std::array<Flags, 9> flags;
            /**
             * Toggles the specified mark's state between enabled/disabled
             */
            Flags &operator[](const int &i);
        };
        /**
         * Initialises the cell empty
         */
        Cell();
        /**
         * Comparison operator
         * Compares the value member
         * If value == 0, or cell is not enabled, it cannot be a match
         */
        bool operator==(const int &other);
        bool operator==(const Cell &other);
        bool operator!=(const int &other);
        bool operator!=(const Cell &other);
        /**
         * Disable all marks
         */
        void clearMarks();
        /**
         * Sets the value of the cell
         * If 0 is passed, the cell is reset
         */
        Cell &operator=(const int &i);
        unsigned char value:4;
        unsigned char wrong:1;
        Marks marks;
    };
    /**
     * Represents a row of 9 cells
     * @note This only exists so we can hijack 2D array operators, to shift 1-indexes to 0-indexes
     */
    struct CellRow {
        /**
         * Returns the corresponding cell
         * 1-indexed
         */
        Cell &operator[](const int &y);
        std::array<Cell, 9> cols;
    };
    /**
     * Basic constructor
     * Selected cell is set as disabled (any out of bounds value)
     */
    Board();
    /**
     * Access a cell at the specified position of the board
     */
    CellRow &operator[](const int &x);
    /**
     * Creates the overlay if it doesn't already exist, and returns it's shared_ptr
     * @param dims If creating the overlay, it will be given these dimensions
     */
    std::shared_ptr<BoardOverlay> getOverlay(const unsigned int &dims = 720);
    /**
     * Kill the overlay, without losing the board state
     * This should be called before GL context is destroyed
     */
    void killOverlay();
    /**
     * States whether overlay exists
     */
    bool hasOverlay() const;
    /**
     * Sets the selected cell
     * Triggers the overlay to update
     */
    void setSelectedCell(const int &x, const int &y);
    void shiftSelectedCell(const int &x, const int &y);
    /**
     * Returns the selected cell
     */
    const Pos &getSelectedCell();
    /**
     * Processes the corresponding number being pressed with the provided modifier keys
     * When number == 0, it is considered delete/reset
     */
    void handleKeyPress(const int &keycode, bool shift, bool ctrl, bool alt);
    /**
     * Validates according to current_mode
     * Clears wrong flag, and newly sets wrong to True for affected cells
     * @note This could be improved (in terms of performance) to perform validation tests on the areas affected by the changed cell
     */
    bool validate();
    /**
     * Clears the wrong flag in all cells
     */
    void clearWrong();
    Mode getMode() const  { return current_mode; }
    void setMode(const Mode &mode);

 private:
    Mode current_mode = Vanilla;
    /**
     * Selected cell, anything out of bounds [1-9][1-9] counts as disabled
     */
    Pos selected_cell;
    /**
     * Never access this directly
     */
    typedef std::array<CellRow, 9> RawBoard;
    RawBoard cell_rows;
    /**
     * The overlay for rendering the board
     */
    std::shared_ptr<BoardOverlay> overlay = nullptr;

    std::stack<RawBoard> undoStack;
    std::stack<RawBoard> redoStack;
};

#endif  // SRC_SUDOKU_BOARD_H_
