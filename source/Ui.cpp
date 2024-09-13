#include "Ui.h"
#include "ThreatDetector.h"
#include <cmath>
#include <cassert>

void Ui::set_bound(Rectangle new_bound) {
    bound = new_bound;
    square_size = std::min(bound.width, bound.height)/(SIZE + 1);
    board_bound.x = bound.x + bound.width/2.0f - (SIZE*square_size)/2.0f - square_size*0.5f;
    board_bound.y = bound.y + bound.height/2.0f - (SIZE*square_size)/2.0f - square_size*0.5f;
    board_bound.width = square_size * SIZE;
    board_bound.height = square_size * SIZE;
    padding = square_size/10.0f;
}

void Ui::render_board(const BitBoard &board) {
    Vector2 mouse = GetMousePosition();
    for (size_t i = 0; i < SIZE; i++) {
        const char* text = TextFormat("%zu", i);
        int text_w = MeasureText(text, font_size);
        Color row_color = i == (size_t)((mouse.y - board_bound.y)/square_size) ? TEXT_HIGHLIGHT_COLOR : TEXT_COLOR;
        Color col_color = i == (size_t)((mouse.x - board_bound.x)/square_size) ? TEXT_HIGHLIGHT_COLOR : TEXT_COLOR;
        DrawText(text,
                 board_bound.x + SIZE*square_size + square_size*0.5f - text_w*0.5f,
                 board_bound.y + i*square_size + square_size*0.5f - font_size*0.5f,
                 font_size, row_color);
        DrawText(text,
                 board_bound.x + i*square_size + square_size*0.5f - text_w*0.5f,
                 board_bound.y + SIZE*square_size + square_size*0.5f - font_size*0.5f,
                 font_size, col_color);
    }
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            Rectangle rec {
                .x = board_bound.x + col*square_size + padding,
                .y = board_bound.y + row*square_size + padding,
                .width = square_size - padding,
                .height = square_size - padding,
            };
            Figure cell = board.get_cell(row, col);
            bool mouse_in_cell = mouse.x > rec.x && mouse.x < rec.x + rec.width && mouse.y > rec.y && mouse.y < rec.y + rec.height;
            if (mouse_in_cell && cell == Figure::None) {
                DrawRectangleRec(rec, MOUSE_IN_EMPTY_COLOR);
            } else {
                DrawRectangleRec(rec, EMPTY_COLOR);
            }
            switch (cell) {
                case Figure::White: {
                    Color color = mouse_in_cell ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
                    if (board.moves.size() > 0 && board.moves.back().pos == Coord(row, col)) {
                        DrawCircle(rec.x + rec.width/2.0f, rec.y + rec.height/2.0f, rec.width/2.0f, LAST_MOVE_COLOR);
                    }
                    DrawCircle(rec.x + rec.width/2.0f, rec.y + rec.height/2.0f, rec.width/2.5f, color);
                } break;
                case Figure::Black: {
                    Color color = mouse_in_cell ? MOUSE_IN_BLACK_COLOR : BLACK_COLOR;
                    if (board.moves.size() > 0 && board.moves.back().pos == Coord(row, col)) {
                        DrawCircle(rec.x + rec.width/2.0f, rec.y + rec.height/2.0f, rec.width/2.0f, LAST_MOVE_COLOR);
                    }
                    DrawCircle(rec.x + rec.width/2.0f, rec.y + rec.height/2.0f, rec.width/2.5f, color);
                } break;
                case Figure::None: break;
                case Figure::Out: break;
            }
        }
    }
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
    Rectangle real_bound {
        .x = board_bound.x,
        .y = board_bound.y,
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
