#include "Ui.h"
#include <cassert>

void render_board(const BitBoard &board) {
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            Cell cell = board.get_cell(row, col);
            assert(cell != Cell::Out && "Invalid cell");
            if (cell == Cell::White) printf("X");
            else if (cell == Cell::Black) printf("O");
            else printf("_");
        }
        printf("\n");
    }
}

void render_line(uint32_t line) {
    for (size_t i = 0; i < RADIUS*2 + 1; i++) {
        switch (Cell((line >> 2*(RADIUS*2-i)) & 0b11)) {
            case Cell::Out: printf("."); break;
            case Cell::None: printf("_"); break;
            case Cell::White: printf("X"); break;
            case Cell::Black: printf("O"); break;
        }
    }
    printf("\n");
}
