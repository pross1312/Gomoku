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
    InitWindow(1300, 800, "Gomoku");
    SetTargetFPS(20);
    Ui ui;

    while (!WindowShouldClose()) {
        ui.bound = Rectangle {
            .x = 0,
            .y = 0,
            .width = (float)GetScreenWidth(),
            .height = (float)GetScreenHeight()
        };
        BeginDrawing();
        ClearBackground(GetColor(0x101010));

        ui.render_board(board);
        auto coord = ui.get_cell_at_pos(GetMousePosition());
        if (coord.has_value()) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        if (coord.has_value() && board.get_cell(coord.value()) == Cell::None &&
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                board.set_cell(coord.value(), Cell::White);
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                board.set_cell(coord.value(), Cell::Black);
            }
            Line4 lines = board.get_lines_radius(coord.value());

            // const auto print_line = [](uint32_t line) {
            //     printf("[");
            //     const size_t size = RADIUS*2-1;
            //     for (size_t i = 0; i < size; i++) {
            //         Cell cell = Cell((line >> 2*(size-1-i)) & 0b11);
            //         printf(cell == Cell::White ? "White" : (cell == Cell::Black ? "Black" : "None"));
            //         if (i+1 < size) printf(", ");
            //     }
            //     printf("]\n");
            // };
            // printf("\nHor: "); print_line(lines.h);
            // printf("Ver: "); print_line(lines.v);
            // printf("Mdg: "); print_line(lines.main_d);
            // printf("Sdg: "); print_line(lines.sub_d);

            TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
                    Threat::to_text(ThreatDetector::check(lines.h)),
                    Threat::to_text(ThreatDetector::check(lines.v)),
                    Threat::to_text(ThreatDetector::check(lines.main_d)),
                    Threat::to_text(ThreatDetector::check(lines.sub_d)));
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
