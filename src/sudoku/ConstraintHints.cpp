#include "sudoku/ConstraintHints.h"

#include <array>

#include "sudoku/Board.h"

namespace ConstraintHints {
namespace {
void columns2(Board &board) {
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Detect any marks which only appear in a specific column
            std::array<std::array<unsigned int, 9>, 3> subcols = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                subcols[x - i * 3 - 1][k - 1]++;
                            }
                        }
                    }
                }
            }
            // Now cleanse the subcols array, removing any marks which appear in multiple columns
            bool hasUseful = false;
            for (int k = 0; k < 9; ++k) {
                unsigned int ct = 0;
                for (int w = 0; w < 3; ++w) {
                    if (subcols[w][k]) {
                        ct++;
                    }
                }
                if (ct > 1) {
                    // Value appear in multiple columns, purge it from subcols array
                    for (int w = 0; w < 3; ++w) {
                        subcols[w][k] = 0;
                    }
                } else if (ct == 1) {
                    hasUseful = true;
                }
            }
            if (hasUseful) {
                // Now we can apply the subcols to all cells in the square column
                for (int _j = 0; _j < 3; ++_j) {
                    // Skip our own square
                    if (_j == j)
                        continue;
                    for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                        for (int y = _j * 3 + 1; y <= _j * 3 + 3; ++y) {
                            Board::Cell &c = board(x, y);
                            if (!c.value) {
                                for (int k = 1; k <= 9; ++k) {
                                    if (subcols[x - i * 3 - 1][k - 1]) {
                                        // c.marks[k].wrong = true;
                                        c.marks[k].enabled = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
void rows2(Board &board) {
    board.transpose();
    columns2(board);
    board.transpose();
}
void loneMarks(Board &board) {
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Count the number of times each mark occurs
            std::array<unsigned int, 9> vals = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                vals[k - 1]++;
                            }
                        }
                    }
                }
            }
            // For each mark which only occurs once
            // Clear other marks from the cell with that mark
            for (int k = 1; k <= 9; ++k) {
                if (vals[k-1] == 1) {
                    // For each cell in the square
                    for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                        for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                            Board::Cell &c = board(x, y);
                            // If this is the cell with the mark
                            if (!c.value && c.marks[k].enabled && !c.marks[k].wrong) {
                                // Set all other marks to disabled
                                for (int _k = 1; _k <= 9; ++_k) {
                                    if (_k != k) {
                                        // c.marks[_k].wrong = true;
                                        c.marks[_k].enabled = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
}  // namespace

void vanilla(Board &board) {
    // First order hints, is the rule broken directly
    columns(board);
    rows(board);
    squares(board);
    // Second order hints, does the impact of a column/row rule on a square (3x3 cell collection)
    // Implicitly prevent a value in a related square
    columns2(board);
    rows2(board);
    // If a mark only appears once in a square, it is the correct value, so remove other marks
    loneMarks(board);
}
void columns(Board &board) {
    // For each column
    for (int x = 1; x <= 9; ++x) {
        std::array<unsigned int, 9> vals = {};
        // Count how many of each value appear in the column
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (c.value) {
                vals[c.value-1]++;
            }
        }
        // Now for each cell, where value is not set disable any marks for values that appear in the array
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (!c.value) {
                for (int i = 1; i <= 9; ++i) {
                    if (vals[i-1]) {
                        // c.marks[i].wrong = true;
                        c.marks[i].enabled = false;
                    }
                }
            }
        }
    }
}
void rows(Board &board) {
    board.transpose();
    columns(board);
    board.transpose();
}
void squares(Board &board) {
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::array<unsigned int, 9> vals = {};
            // Count how many of each value appear in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (c.value) {
                        vals[c.value-1]++;
                    }
                }
            }
            // Now for each cell, where value is not set disable any marks for values that appear in the array
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        for (int k = 1; k <= 9; ++k) {
                            if (vals[k-1]) {
                                // c.marks[k].wrong = true;
                                c.marks[k].enabled = false;
                            }
                        }
                    }
                }
            }
        }
    }
}
}  // namespace ConstraintHints