#include "Ui.h"
#include "ThreatDetector.h"
#include <cmath>
#include <cassert>

void Ui::render_board(const BitBoard &board) {
    const float square_size = std::min(bound.width, bound.height)/SIZE;
    const float padding = square_size/10.0f;
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            Rectangle rec {
                .x = bound.x + bound.width/2.0f - (SIZE*square_size)/2.0f + col*square_size + padding,
                .y = bound.y + bound.height/2.0f - (SIZE*square_size)/2.0f + row*square_size + padding,
                .width = square_size - padding,
                .height = square_size - padding,
            };
            Figure cell = board.get_cell(row, col);
            Vector2 mouse = GetMousePosition();
            bool mouse_in_cell = mouse.x > rec.x && mouse.x < rec.x + rec.width && mouse.y > rec.y && mouse.y < rec.y + rec.height;
            if (mouse_in_cell && cell == Figure::None) {
                DrawRectangleRec(rec, MOUSE_IN_EMPTY_COLOR);
            } else {
                DrawRectangleRec(rec, EMPTY_COLOR);
            }
            switch (cell) {
                case Figure::White: render_white(rec, mouse_in_cell); break;
                case Figure::Black: render_black(rec, mouse_in_cell); break;
                case Figure::None: break;
                case Figure::Out: break;
            }
        }
    }
}

void Ui::render_white(Rectangle box, bool mouse_in) {
//     const float padding = 5.0f;
//     Color color = mouse_in ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
//     DrawLineEx(Vector2{.x = box.x + padding, .y = box.y + padding},
//                Vector2{.x = box.x + box.width - padding, .y = box.y - padding + box.height}, 10.0f, color);
//     DrawLineEx(Vector2{.x = box.x + box.width - padding, .y = box.y + padding},
//                Vector2{.x = box.x + padding, .y = box.y - padding + box.height}, 10.0f, color);
    Color color = mouse_in ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
    DrawCircle(box.x + box.width/2.0f, box.y + box.height/2.0f, box.width/2.2f, color);
}

void Ui::render_black(Rectangle box, bool mouse_in) {
    Color color = mouse_in ? MOUSE_IN_BLACK_COLOR : BLACK_COLOR;
    DrawCircle(box.x + box.width/2.0f, box.y + box.height/2.0f, box.width/2.2f, color);
}

// Ui::void render_board(const BitBoard &board) {
//     for (size_t row = 0; row < SIZE; row++) {
//         for (size_t col = 0; col < SIZE; col++) {
//             Figure cell = board.get_cell(row, col);
//             assert(cell != Figure::Out && "Invalid cell");
//             if (cell == Figure::White) printf("X");
//             else if (cell == Figure::Black) printf("O");
//             else printf("_");
//         }
//         printf("\n");
//     }
// }
// 
void Ui::render_line(uint32_t line) {
    for (size_t i = 0; i < RADIUS*2 + 1; i++) {
        switch (Figure((line >> 2*(RADIUS*2-i)) & 0b11)) {
            case Figure::Out: printf("."); break;
            case Figure::None: printf("_"); break;
            case Figure::White: printf("X"); break;
            case Figure::Black: printf("O"); break;
        }
    }
    printf("\n");
}

std::optional<Coord> Ui::get_cell_at_pos(Vector2 pos) {
    const float square_size = std::min(bound.width, bound.height)/SIZE;
    const float padding = square_size/10.0f;
    Rectangle real_bound {
        .x = bound.x + bound.width/2.0f - (SIZE*square_size)/2.0f,
        .y = bound.y + bound.height/2.0f - (SIZE*square_size)/2.0f,
        .width = square_size*SIZE,
        .height = square_size*SIZE,
    };
    int col = (pos.x - real_bound.x)/square_size;
    int row = (pos.y - real_bound.y)/square_size;
    if (col < 0 || col >= SIZE || row < 0 || row >= SIZE) return std::nullopt;
    Rectangle rec {
        .x = real_bound.x + col*square_size + padding,
        .y = real_bound.y + row*square_size + padding,
        .width = square_size - padding,
        .height = square_size - padding,
    };
    bool mouse_in_cell = pos.x > rec.x && pos.x < rec.x + rec.width && pos.y > rec.y && pos.y < rec.y + rec.height;
    if (mouse_in_cell) return Coord((size_t)row, (size_t)col);
    return std::nullopt;
}
