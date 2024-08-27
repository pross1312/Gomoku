#include "BitBoard.h"
#include "ThreatDetector.h"
#include "Ui.h"
#include <raylib.h>
#include <ctime>
#include <cstdlib>

#define print_bin(x) do {                                \
    for (size_t _i = 1; _i <= sizeof((x))*8; _i++) {     \
        printf("%d", ((x) >> ((sizeof((x))*8) - _i))&1); \
    }                                                    \
    printf("\n");                                        \
} while(false);

int main() {
    srand(time(NULL));
    BitBoard board;
    for (size_t i = 0; i < SIZE; i++) {
        for (size_t j = 0; j < SIZE; j++) {
            board.set_cell(i, j, Cell((rand()%3)+1));
        }
    }
    InitWindow(1300, 800, "Gomoku");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x101010));
        bool mouse_in = render_board(board);
        if (mouse_in) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        EndDrawing();
    }
    CloseWindow();
    // board.set_cell(0, 0, Cell::Black);
    // board.set_cell(1, 1, Cell::Black);
    // board.set_cell(2, 2, Cell::Black);
    // board.set_cell(3, 3, Cell::Black);
    // board.set_cell(4, 4, Cell::Black);
    // board.set_cell(5, 5, Cell::White);
    // ThreatDetector detector;
    // Line4 lines = board.get_lines_radius(4, 4);
    // render_board(board);
    // render_line(lines.sub_d);
    // printf("%d, ", detector.check(lines.h));
    // printf("%d, ", detector.check(lines.v));
    // printf("%d, ", detector.check(lines.main_d));
    // printf("%d\n", detector.check(lines.sub_d));
    return 0;
}
