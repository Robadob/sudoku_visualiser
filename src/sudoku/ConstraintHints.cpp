#include "sudoku/ConstraintHints.h"

#include <array>
#include <list>

#include "sudoku/Board.h"

namespace ConstraintHints {
namespace {
/**
 * Use common method for setting marks wrong, so we can change the effect in one place
 * @param c The affected cell
 * @param i The mark index
 */
void setMarkWrong(Board::Cell &c, const int &i) {
    // c.marks[i].wrong = true;
    c.marks[i].enabled = false;
}
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
                                        setMarkWrong(c, k);
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
void naked_frequency(Board &board, const int &i, const int &j, const std::array<std::list<Board::Pos>, 9> &mark_occurrences, const unsigned int &frequency) {
    for (int k = 1; k < 9; ++k) {
        if (mark_occurrences[k - 1].size() == frequency) {
            // Create cell tc, the union of cells in mark_occurrences[k - 1]
            Board::Cell tc;
            unsigned int tc_marks = 0;
            // For each cell and mark, perform union
            // Check that 1 cells has frequency marks, and the mark not k is also in other cell
            for (int _k = 1; _k <= 9; ++_k) {
                for (const auto &p : mark_occurrences[k - 1]) {
                    Board::Cell &c = board(p);
                    tc.marks[_k].enabled = tc.marks[_k].enabled || (c.marks[_k].enabled && !c.marks[_k].wrong);
                }
                if (tc.marks[_k].enabled)
                    ++tc_marks;
            }
            // If tc only has frequency marks
            if (tc_marks == frequency) {
                // Disable those 2 marks in all other cells in square
                for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                    for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                        for (const auto &p : mark_occurrences[k - 1]) {
                            if (x == p.x && y == p.y) {
                                goto naked_frequency_skip_disable;
                            }
                        }
                        Board::Cell &c = board(x, y);
                        if (!c.value) {
                            for (int _k = 1; _k <= 9; ++_k) {
                                if (tc.marks[_k].enabled && c.marks[_k].enabled && !c.marks[_k].wrong) {
                                    setMarkWrong(c, _k);
                                }
                            }
                        }
                        {naked_frequency_skip_disable:;}
                    }
                }
            }
        }
    }
}
void nakedDoubles(Board &board) {
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Detect how many cells each mark appears in
            std::array<std::list<Board::Pos>, 9> mark_occurrences = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                mark_occurrences[k - 1].push_back({x, y});
                            }
                        }
                    }
                }
            }
            // If any mark appears frequency times
            // perform the union of marks of all cells it appears in
            // If the union only has frequency marks enabled, purge these marks from all other cells
            naked_frequency(board, i, j, mark_occurrences, 2);
        }
    }
}
void nakedTriples(Board &board) {
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Detect how many cells each mark appears in
            std::array<std::list<Board::Pos>, 9> mark_occurrences = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                mark_occurrences[k - 1].push_back({x, y});
                            }
                        }
                    }
                }
            }
            // If any mark appears frequency times
            // perform the union of marks of all cells it appears in
            // If the union only has frequency marks enabled, purge these marks from all other cells
            naked_frequency(board, i, j, mark_occurrences, 3);
        }
    }
}
void hiddenSingles(Board &board) {
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
                                        setMarkWrong(c, _k);
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
void hiddenDoubles(Board &board) {
    const unsigned int frequency = 2;
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Count the number of times each mark occurs
            std::array<unsigned int, 9> mark_frequency = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                mark_frequency[k-1]++;
                            }
                        }
                    }
                }
            }
            // For each mark value, which occurs twice
            for (int k1 = 1; k1 <= 9; ++k1) {
                if (mark_frequency[k1-1] == frequency) {
                    // Find the 2nd mark with a matching frequency
                    for (int k2 = k1 + 1; k2 <= 9; ++k2) {
                        if (mark_frequency[k2-1] == frequency) {
                            // Check whether their marks appear in the same cells
                            std::list<Board::Pos> matching_marks;
                            // For each cell in the square
                            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                                    Board::Cell &c = board(x, y);
                                    if (!c.value) {
                                        if (c.marks[k1].enabled && !c.marks[k1].wrong &&
                                            c.marks[k2].enabled && !c.marks[k2].wrong) {
                                            matching_marks.emplace_back(Board::Pos(x, y));
                                        }
                                    }
                                }
                            }
                            if (matching_marks.size() == frequency) {
                                // We found a hidden pair
                                // Purge other marks from the affected cells
                                // For each cell in the square
                                for (auto &mm : matching_marks) {
                                    Board::Cell &c = board(mm);
                                    if (!c.value) {
                                        // Clear marks in all but k1 and k2
                                        for (int k = 1; k <= 9; ++k) {
                                            if (k != k1 && k != k2) {
                                                setMarkWrong(c, k);
                                            }
                                        }
                                    }
                                }
                                // We can skip reset of current k1 loop now
                                goto double_outer_mark_freq_loop;
                            }
                        }
                    }
                }
                {double_outer_mark_freq_loop:;}
            }
        }
    }
}
void hiddenTriples(Board &board) {
    const unsigned int frequency = 3;
    // For each square
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Count the number of times each mark occurs
            std::array<unsigned int, 9> mark_frequency = {};
            // For each cell in the square
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.value) {
                        // Increment the subcolumn counter for any marks that are set
                        for (int k = 1; k <= 9; ++k) {
                            if (c.marks[k].enabled && !c.marks[k].wrong) {
                                mark_frequency[k-1]++;
                            }
                        }
                    }
                }
            }
            // For each mark value, with correct frequency
            for (int k1 = 1; k1 <= 9; ++k1) {
                if (mark_frequency[k1-1] == frequency) {
                    // Find the 2nd mark with a matching frequency
                    for (int k2 = k1 + 1; k2 <= 9; ++k2) {
                        if (mark_frequency[k2-1] == frequency) {
                            // Find the 3rd mark with a matching frequency
                            for (int k3 = k2 + 1; k3 <= 9; ++k3) {
                                if (mark_frequency[k3-1] == frequency) {
                                    // Check whether their marks appear in the same cells
                                    std::list<Board::Pos> matching_marks;
                                    // For each cell in the square
                                    for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                                        for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                                            Board::Cell &c = board(x, y);
                                            if (!c.value) {
                                                if (c.marks[k1].enabled && !c.marks[k1].wrong &&
                                                    c.marks[k2].enabled && !c.marks[k2].wrong &&
                                                    c.marks[k3].enabled && !c.marks[k3].wrong) {
                                                    matching_marks.emplace_back(Board::Pos(x, y));
                                                }
                                            }
                                        }
                                    }
                                    if (matching_marks.size() == frequency) {
                                        // We found a hidden triple
                                        // Purge other marks from the affected cells
                                        // For each cell in the square
                                        for (auto &mm : matching_marks) {
                                            Board::Cell &c = board(mm);
                                            if (!c.value) {
                                                // Clear marks in all but k1 and k2
                                                for (unsigned int k = 1; k <= 9; ++k) {
                                                    if (k != k1 && k != k2 && k != k3) {
                                                        setMarkWrong(c, k);
                                                    }
                                                }
                                            }
                                        }
                                        // We can skip reset of current k1 loop now
                                        goto triple_outer_mark_freq_loop;
                                    }
                                }
                            }
                        }
                    }
                }
                {triple_outer_mark_freq_loop:;}
            }
        }
    }
}
void yWing(Board &board) {
    // For all cells
    for (int x = 1; x <= 9; ++x) {
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (!c.value) {
                // Count marks
                std::list<unsigned int> marks;
                for (int k = 1; k <= 9; ++k) {
                    if (c.marks[k].enabled && !c.marks[k].wrong) {
                        marks.push_back(k);
                    }
                }
                // If only 2 marks
                if (marks.size() == 2) {
                    auto it = marks.begin();
                    unsigned int mark1 = *it;
                    unsigned int mark2 = *(++it);
                    // Fork the board with each mark set, and solve it with each of these marks set
                    Board board1(board);
                    Board board2(board);
                    board1(x, y) = mark1;
                    board2(x, y) = mark2;
                    board1.hint(true);
                    board2.hint(true);
                    bool chainSuccess = false;
                    // Review all changed marks in both boards.
                    for (int _x = 1; _x <= 9; ++_x) {
                        for (int _y = 1; _y <= 9; ++_y) {
                            Board::Cell &c0 = board(_x, _y);
                            Board::Cell &c1 = board1(_x, _y);
                            Board::Cell &c2 = board2(_x, _y);
                            if (!c0.value) {
                                std::list<unsigned int> missing_marks;
                                for (int k = 1; k <= 9; ++k) {
                                    const bool mark_set0 = c0.marks[k].enabled && !c0.marks[k].wrong;
                                    const bool mark_set1 = c1.marks[k].enabled && !c1.marks[k].wrong;
                                    const bool mark_set2 = c2.marks[k].enabled && !c2.marks[k].wrong;
                                    if (mark_set0 && !mark_set1 && !mark_set2) {
                                        missing_marks.push_back(k);
                                    }
                                }
                                if (!missing_marks.empty()) {
                                    chainSuccess = true;
                                }
                                // We can set any marks in missing_marks as wrong
                                for (const auto &mm : missing_marks) {
                                    setMarkWrong(c0, mm);
                                }
                            }
                        }
                    }
                    if (chainSuccess) {
                        return;  // Only do 1 useful chain before returning to normal rules
                    }
                }
            }
        }
    }
}
void xWingColumn(Board &board) {
    // For all columns
    for (int x = 1; x <= 9; ++x) {
        // Count mark frequencies
        std::array<std::list<Board::Pos>, 9> marks_occurences = {};
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (!c.value) {
                for (int k = 1; k <= 9; ++k) {
                    if (c.marks[k].enabled && !c.marks[k].wrong) {
                        marks_occurences[k - 1].push_back({x, y});
                    }
                }
            }
        }
        for (int k = 1; k <= 9; ++k) {
            // If only twice
            if (marks_occurences[k - 1].size() == 2) {
                auto it = marks_occurences[k - 1].begin();
                Board::Pos pos1 = *it;
                Board::Pos pos2 = *(++it);
                // Fork the board with each mark set, and solve it with each of these marks set
                Board board1(board);
                Board board2(board);
                board1(pos1) = k;
                board2(pos2) = k;
                board1.hint(true);
                board2.hint(true);
                bool chainSuccess = false;
                // Review all changed marks in both boards.
                for (int _x = 1; _x <= 9; ++_x) {
                    for (int _y = 1; _y <= 9; ++_y) {
                        Board::Cell &c0 = board(_x, _y);
                        Board::Cell &c1 = board1(_x, _y);
                        Board::Cell &c2 = board2(_x, _y);
                        if (!c0.value) {
                            std::list<unsigned int> missing_marks;
                            for (int _k = 1; _k <= 9; ++_k) {
                                const bool mark_set0 = c0.marks[_k].enabled && !c0.marks[_k].wrong;
                                const bool mark_set1 = c1.marks[_k].enabled && !c1.marks[_k].wrong;
                                const bool mark_set2 = c2.marks[_k].enabled && !c2.marks[_k].wrong;
                                if (mark_set0 && !mark_set1 && !mark_set2) {
                                    missing_marks.push_back(_k);
                                }
                            }
                            if (!missing_marks.empty()) {
                                chainSuccess = true;
                            }
                            // We can set any marks in missing_marks as wrong
                            for (const auto &mm : missing_marks) {
                                setMarkWrong(c0, mm);
                            }
                        }
                    }
                }
                if (chainSuccess) {
                    return;  // Only do 1 useful chain before returning to normal rules
                }
            }
        }
    }
}
void xWingRow(Board &board) {
    board.transpose();
    xWingColumn(board);
    board.transpose();
}
}  // namespace

void vanilla(Board &board, const bool &skip_chaining) {
    Board::RawBoard prev_raw_board = {};
    do {
        do {
            do {
                do {
                    prev_raw_board = board.getRawBoard();
                    // First order hints, is the rule broken directly
                    columns(board);
                    rows(board);
                    squares(board);
                    // Pointing pair columns/rows
                    // Second order hints, does the impact of a column/row rule on a square (3x3 cell collection)
                    // Implicitly prevent a value in a related square
                    columns2(board);
                    rows2(board);
                    // Naked doubles/triples
                    // Double: If a mark only appears in 2 cells, with only the same 1 mark, that mark can be removed from other cells in the square
                    // Triple: If a mark only appears in 3 cells, with only the same 2 marks, those 2 marks can be removed from other cells in the square
                    nakedDoubles(board);
                    nakedTriples(board);
                    // If a mark only appears once in a square, it is the correct value, so remove other marks
                    hiddenSingles(board);
                    // If two marks only appear twice in a square, and they appear in the same cells, remove other marks from these cells
                    hiddenDoubles(board);
                    // Same pattern as hiddenSingles(), hiddenDoubles() but for triples
                    hiddenTriples(board);
                } while (prev_raw_board != board.getRawBoard());
                if (!skip_chaining) {
                    // Chaining
                    // For every cell with only 2 marks, fork the board with the two possibilities
                    // Run hint with chaining disabled
                    // Only retain marks which appear in the union of the two boards
                    yWing(board);
                }
            } while (prev_raw_board != board.getRawBoard());
            if (!skip_chaining) {
                // For every column where a mark only appears twice
                xWingColumn(board);
            }
        } while (prev_raw_board != board.getRawBoard());
        if (!skip_chaining) {
            // For every row where a mark only appears twice
            xWingRow(board);
        }
    } while (prev_raw_board != board.getRawBoard());
}
void columns(Board &board) {
    // For each column
    for (int x = 1; x <= 9; ++x) {
        std::array<unsigned int, 9> vals = {};
        // Count how many of each value appear in the column
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            unsigned char val = c.rawValue();
            if (val) {
                vals[val-1]++;
            }
        }
        // Now for each cell, where value is not set disable any marks for values that appear in the array
        for (int y = 1; y <= 9; ++y) {
            Board::Cell &c = board(x, y);
            if (!c.rawValue()) {
                for (int i = 1; i <= 9; ++i) {
                    if (vals[i-1]) {
                        setMarkWrong(c, i);
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
                    unsigned char val = c.rawValue();
                    if (val) {
                        vals[val-1]++;
                    }
                }
            }
            // Now for each cell, where value is not set disable any marks for values that appear in the array
            for (int x = i * 3 + 1; x <= i * 3 + 3; ++x) {
                for (int y = j * 3 + 1; y <= j * 3 + 3; ++y) {
                    Board::Cell &c = board(x, y);
                    if (!c.rawValue()) {
                        for (int k = 1; k <= 9; ++k) {
                            if (vals[k-1]) {
                                setMarkWrong(c, k);
                            }
                        }
                    }
                }
            }
        }
    }
}
}  // namespace ConstraintHints
