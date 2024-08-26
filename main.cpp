#include "BitBoard.h"
#include "Ui.h"
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
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            board.set_cell(row, col, Cell(rand()%3 + 1));
        }
    }
    render_board(&board);
    Line4 lines = board.get_lines_radius(7, 7);
    printf("\n");
    render_line(lines.h);
    render_line(lines.v);
    render_line(lines.main_d);
    render_line(lines.sub_d);
    return 0;
}
