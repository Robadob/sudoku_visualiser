#ifndef SRC_SUDOKU_CONSTRAINTHINTS_H_
#define SRC_SUDOKU_CONSTRAINTHINTS_H_

class Board;

/**
 * Collection of static methods for automatically setting marks to hint the user
 * Mostly here to save further cluttering Board
 */
namespace ConstraintHints {
    /**
     * Vanilla sudoku rules
     * Combines columns AND rows AND squares
     */
    void vanilla(Board &board);
    void columns(Board &board);
    void rows(Board &board);
    void squares(Board &board);
}  // namespace ConstraintHints

#endif  // SRC_SUDOKU_CONSTRAINTHINTS_H_
