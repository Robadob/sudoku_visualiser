#ifndef SRC_SUDOKU_BOARD_H_
#define SRC_SUDOKU_BOARD_H_

#include <memory>

#include "sudoku/BoardOverlay.h"

class Board {
 public:
    /**
     * Represents a single number that can be written into a sudoku board
     */
    struct Cell {
        struct Flags {
            unsigned char enabled:1;
            unsigned char wrong:1;
        };
        /**
         * The mini numbers within a cell
         * Only visible when the value for the cell is not set
         */
        struct Marks {
            Flags flags[9];
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
         * Comparses the value member
         * If value == 0, or cell is not enabled, it cannot be a match
         */
        bool operator==(const int &other);
        bool operator==(const Cell &other);
        bool operator!=(const int &other);
        bool operator!=(const Cell &other);
        /**
         * Sets the value of the cell
         * If 0 is passed, the cell is reset
         */
        Cell &operator=(const int &i);
        unsigned char value:4;
        Flags flags;
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
        Cell cols[9];
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
    const glm::ivec2 &getSelectedCell();
    /**
     * Processes the corresponding number being pressed with the provided modifier keys
     * When number == 0, it is considered delete/reset
     */
    void handleNumberPress(const int &number, bool shift, bool ctrl, bool alt);

 private:
    /**
     * Selected cell, anything out of bounds [1-9][1-9] counts as disabled
     */
    glm::ivec2 selected_cell;
    /**
     * Never access this directly
     */
    CellRow cell_rows[9];
    /**
     * The overlay for rendering the board
     */
    std::shared_ptr<BoardOverlay> overlay = nullptr;
};

#endif  // SRC_SUDOKU_BOARD_H_
