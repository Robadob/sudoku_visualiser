#include "sudoku/ConstraintValidator.h"

#include <array>

#include "sudoku/Board.h"

namespace ConstraintValidator {
bool vanilla(Board &board) {
    // Need to make sure all 3 are evaluated before we return
    // Otherwise we might not mark all wrong cells!
    const glm::bvec3 wrong(columns(board), rows(board), squares(board));
    return any(wrong);
}
bool columns(Board &board) {
    // Note wrong count does not consider the first cell found with a value as wrong
    unsigned int wrongCount = 0;
    // For each column
    for (int x = 1; x <= 9; ++x) {
        std::array<Board::Pos, 9> vals;
        // Assign first value found to it's location in the array
        // If the array value has already been set, mark both as wrong
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (c.value) {
                Board::Pos &loc = vals[c.value-1];
                if (loc) {
                    // Location is already set
                    c.wrong = true;
                    board(loc.x, loc.y).wrong = true;
                    wrongCount++;
                } else {
                    // Location is fresh
                    loc = {x, y};
                }
            }
        }
    }
    return !wrongCount;
}
bool rows(Board &board) {
    board.transpose();
    bool rtn = columns(board);
    board.transpose();
    return rtn;
}
bool squares(Board &board) {
    // Note wrong count does not consider the first cell found with a value as wrong
    unsigned int wrongCount = 0;
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::array<Board::Pos, 9> vals;
            // Assign first value found to it's location in the array
            // If the array value has already been set, mark both as wrong
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (c.value) {
                        Board::Pos &loc = vals[c.value-1];
                        if (loc) {
                            // Location is already set
                            c.wrong = true;
                            board(loc.x, loc.y).wrong = true;
                            wrongCount++;
                        } else {
                            // Location is fresh
                            loc = {x, y};
                        }
                    }
                }
            }
        }
    }
    return !wrongCount;
}
}  // namespace ConstraintValidator
