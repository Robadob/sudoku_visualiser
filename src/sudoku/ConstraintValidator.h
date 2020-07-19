#ifndef SRC_SUDOKU_CONSTRAINTVALIDATOR_H_
#define SRC_SUDOKU_CONSTRAINTVALIDATOR_H_

class Board;

/**
 * Collection of static methods for validating a board against different constraints, all take a board and return true or false.
 * Mostly here to save further cluttering Board
 */
namespace ConstraintValidator {
    /**
     * Vanilla sudoku rules
     * Combines columns AND rows AND squares
     */
    bool vanilla(Board &board);
    bool columns(Board &board);
    bool rows(Board &board);
    bool squares(Board &board);
}  // namespace ConstraintValidator

#endif  // SRC_SUDOKU_CONSTRAINTVALIDATOR_H_
