#ifndef SRC_SUDOKU_CONSTRAINTHINTS_H_
#define SRC_SUDOKU_CONSTRAINTHINTS_H_

class Board;

/**
 * Collection of static methods for automatically setting marks to hint the user
 * Mostly here to save further cluttering Board
 */
namespace ConstraintHints {
struct VanillaConstraints {
    bool columns = true;
    bool rows = true;
    bool squares = true;
    bool pointingPairColumns = true;
    bool pointingPairRows = true;
    bool nakedDoubles = true;
    bool nakedTriples = true;
    bool hiddenSingles = true;
    bool hiddenDoubles = true;
    bool hiddenTriples = true;
    bool yWing = false;
    bool xWingColumn = false;
    bool xWingRow = false;
};
/**
 * Vanilla sudoku rules
 * Combines columns AND rows AND squares
 */
void vanilla(Board &board, const VanillaConstraints &constraints = {});
void columns(Board &board);
void rows(Board &board);
void squares(Board &board);
}  // namespace ConstraintHints

#endif  // SRC_SUDOKU_CONSTRAINTHINTS_H_
