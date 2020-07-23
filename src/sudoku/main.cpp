#include "Visualiser.h"
#include "sudoku/Board.h"

int main(int argc, char **argv) {
    // Create Sudoku visualiser
    Visualiser vis;
    // Create the window and set it rendering in background thread
    vis.start();

    {
        // Get the pointer to the board
        std::shared_ptr<Board> sudoku_board = vis.getBoard();

        // Init all marks as on
        for (int x = 1; x <= 9; ++x) {
            for (int y = 1; y <= 9; ++y) {
                for (int i = 1; i <= 9; ++i)
                    (*sudoku_board)(x, y).marks[i].enabled = true;
            }
        }
        sudoku_board->getOverlay()->queueRedrawAllCells();
    }

    // Join the background thread
    // (This leaves the visualisation running until the window is closed)
    vis.join();
}
