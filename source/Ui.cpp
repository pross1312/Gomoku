#include "Ui.h"
#include <cassert>

bool render_board(const BitBoard &board) {
    bool mouse_in_any = false;
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            Rectangle rec {
                .x = GetScreenWidth()/2.0f - (SIZE*CELL_SIZE)/2.0f + col*CELL_SIZE + PADDING,
                .y = GetScreenHeight()/2.0f - (SIZE*CELL_SIZE)/2.0f + row*CELL_SIZE + PADDING,
                .width = CELL_SIZE - PADDING,
                .height = CELL_SIZE - PADDING,
            };
            Vector2 mouse = GetMousePosition();
            bool mouse_in_rec = mouse.x > rec.x && mouse.x < rec.x + rec.width && mouse.y > rec.y && mouse.y < rec.y + rec.height;
            mouse_in_any = mouse_in_any | mouse_in_rec;
            if (mouse_in_rec && board.get_cell(row, col) == Cell::None) {
                DrawRectangleRec(rec, MOUSE_IN_EMPTY_COLOR);
            } else {
                DrawRectangleRec(rec, EMPTY_COLOR);
            }
            switch (board.get_cell(row, col)) {
                case Cell::White: render_white(rec, mouse_in_rec); break;
                case Cell::Black: render_black(rec, mouse_in_rec); break;
                case Cell::None: break;
                case Cell::Out: break;
            }
        }
    }
    return mouse_in_any;
}

void render_white(Rectangle box, bool mouse_in) {
    const float padding = 5.0f;
    Color color = mouse_in ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
    DrawLineEx(Vector2{.x = box.x + padding, .y = box.y + padding},
               Vector2{.x = box.x + box.width - padding, .y = box.y - padding + box.height}, 10.0f, color);
    DrawLineEx(Vector2{.x = box.x + box.width - padding, .y = box.y + padding},
               Vector2{.x = box.x + padding, .y = box.y - padding + box.height}, 10.0f, color);
}

void render_black(Rectangle box, bool mouse_in) {
    Vector2 mouse = GetMousePosition();
    Color color = mouse_in ? MOUSE_IN_BLACK_COLOR : BLACK_COLOR;
    DrawCircle(box.x + box.width/2.0f, box.y + box.height/2.0f, box.width/2.0f, color);
}

// void render_board(const BitBoard &board) {
//     for (size_t row = 0; row < SIZE; row++) {
//         for (size_t col = 0; col < SIZE; col++) {
//             Cell cell = board.get_cell(row, col);
//             assert(cell != Cell::Out && "Invalid cell");
//             if (cell == Cell::White) printf("X");
//             else if (cell == Cell::Black) printf("O");
//             else printf("_");
//         }
//         printf("\n");
//     }
// }
// 
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
