#include "sudoku/ConstraintValidator.h"

#include <array>

#include "sudoku/Board.h"

namespace ConstraintValidator {
namespace {
struct Pos {
    int x = 0, y = 0;
    /**
     * Returns true if the position is valid
     */
    explicit operator bool() const {
        return x >= 1 && x <= 9 && y >= 1 && y <= 9;
    }
};
}  // namespace
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
        std::array<Pos, 9> vals;
        // Assign first value found to it's location in the array
        // If the array value has already been set, mark both as wrong
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board[x][y];
            if (c.value && c.flags.enabled) {
                Pos &loc = vals[c.value-1];
                if (loc) {
                    // Location is already set
                    c.flags.wrong = true;
                    board[loc.x][loc.y].flags.wrong = true;
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
    // Note wrong count does not consider the first cell found with a value as wrong
    unsigned int wrongCount = 0;
    // For each row
    for (int y = 1; y <= 9; ++y) {
        std::array<Pos, 9> vals;
        // Assign first value found to it's location in the array
        // If the array value has already been set, mark both as wrong
        for (int x = 1; x <= 9; ++x) {
            Board::Cell &c = board[x][y];
            if (c.value && c.flags.enabled) {
                Pos &loc = vals[c.value-1];
                if (loc) {
                    // Location is already set
                    c.flags.wrong = true;
                    board[loc.x][loc.y].flags.wrong = true;
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
bool squares(Board &board) {
    // Note wrong count does not consider the first cell found with a value as wrong
    unsigned int wrongCount = 0;
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::array<Pos, 9> vals;
            // Assign first value found to it's location in the array
            // If the array value has already been set, mark both as wrong
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board[x][y];
                    if (c.value && c.flags.enabled) {
                        Pos &loc = vals[c.value-1];
                        if (loc) {
                            // Location is already set
                            c.flags.wrong = true;
                            board[loc.x][loc.y].flags.wrong = true;
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
