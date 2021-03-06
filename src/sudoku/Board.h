#ifndef SRC_SUDOKU_BOARD_H_
#define SRC_SUDOKU_BOARD_H_

#include <memory>
#include <string>
#include <array>
#include <stack>
#include <utility>

#include "sudoku/BoardOverlay.h"

class Visualiser;

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
            /**
             * Toggles the specified mark's state between enabled/disabled
             */
            Flags &operator[](const int &i);
            bool operator==(const Marks &other) const;
            bool operator!=(const Marks &other) const;

         private:
            std::array<Flags, 9> flags = {};
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
        bool operator==(const int &other) const;
        bool operator==(const Cell &other) const;
        bool operator!=(const int &other) const;
        bool operator!=(const Cell &other) const;
        /**
         * Disable all marks
         */
        void clearMarks();
        /**
         * Set marks to enabled=true, wrong=false, if the cell's value is 0
         */
        void setMarks();
        /**
         * If value != 0, return value
         * else if only 1 mark is enabled, return that mark
         * else return 0;
         */
        unsigned char rawValue();
        /**
         * Sets the value of the cell
         * If 0 is passed, the cell is reset
         */
        Cell &operator=(const unsigned int &i);
        unsigned char value:4;
        unsigned char wrong:1;
        Marks marks;
    };
    typedef std::array<std::array<Cell, 9>, 9> RawBoard;
    /**
     * Basic constructor
     * Selected cell is set as disabled (any out of bounds value)
     */
    explicit Board(Visualiser *vis = nullptr);
    /**
     * Copy constructor
     * Only copies raw board and mode, default init everything else
     */
    Board(const Board &other);
    /**
     * Access a cell at the specified position of the board
     */
    Cell &operator()(const int &x, const int &y);
    Cell &operator()(const Pos &xy);
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
     * Update marks for all unset cells to hint at what is possible/impossible
     */
    void hint(const bool &skipChaining = false);
    /**
     * Set all cells empty
     */
    void clear();
    /**
     * Clears the wrong flag in all cells
     */
    void clearWrong();
    Mode getMode() const  { return current_mode; }
    void setMode(const Mode &mode);

    void transpose() { transposeState = !transposeState; }
    bool getTransposeState() const { return transposeState; }
    RawBoard getRawBoard() const { return raw_board; }
    /**
     * Saves raw_board to "saves/<slot>.bsdk
     * @return True if successful
     */
    bool save(const std::string &slot) const;
    /**
     * Loads raw_board from "saves/<slot>.bsdk
     * @return True if successful
     */
    bool load(const std::string &slot);

 private:
    Mode current_mode = Vanilla;
    /**
     * Selected cell, anything out of bounds [1-9][1-9] counts as disabled
     */
    Pos selected_cell;
    /**
     * Never access this directly
     */
    RawBoard raw_board;
    /**
     * The overlay for rendering the board
     */
    std::shared_ptr<BoardOverlay> overlay = nullptr;
    /**
     * Last result from validate()
     * True means no detected failures
     */
    bool lastValidateResult = true;
    /**
     * If set true, the array accesses are swapped
     */
    bool transposeState = false;
    std::stack<RawBoard> undoStack;
    std::stack<RawBoard> redoStack;
    /**
     * Used for sending notifications to vis if available
     */
    Visualiser *visualiser = nullptr;
};

#endif  // SRC_SUDOKU_BOARD_H_
