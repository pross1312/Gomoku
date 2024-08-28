#include "Ui.h"
#include "ThreatDetector.h"
#include <cmath>
#include <cassert>

void render_board(const BitBoard &board, const Rectangle bound) {
    bool mouse_in_any = false;
    const float cell_size = std::min(bound.width, bound.height)/SIZE;
    const float padding = cell_size/10.0f;
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            Rectangle rec {
                .x = bound.x + bound.width/2.0f - (SIZE*cell_size)/2.0f + col*cell_size + padding,
                .y = bound.y + bound.height/2.0f - (SIZE*cell_size)/2.0f + row*cell_size + padding,
                .width = cell_size - padding,
                .height = cell_size - padding,
            };
            Cell cell = board.get_cell(row, col);
            Vector2 mouse = GetMousePosition();
            bool mouse_in_rec = mouse.x > rec.x && mouse.x < rec.x + rec.width && mouse.y > rec.y && mouse.y < rec.y + rec.height;
            mouse_in_any = mouse_in_any | (mouse_in_rec && (cell == Cell::None));
            if (mouse_in_rec && cell == Cell::None) {
                DrawRectangleRec(rec, MOUSE_IN_EMPTY_COLOR);
            } else {
                DrawRectangleRec(rec, EMPTY_COLOR);
            }
            if (mouse_in_rec && cell != Cell::None && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Line4 lines = board.get_lines_radius(row, col);
                TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
                        Threat::to_text(ThreatDetector::check(lines.h)),
                        Threat::to_text(ThreatDetector::check(lines.v)),
                        Threat::to_text(ThreatDetector::check(lines.main_d)),
                        Threat::to_text(ThreatDetector::check(lines.sub_d)));
            }
            switch (cell) {
                case Cell::White: render_white(rec, mouse_in_rec); break;
                case Cell::Black: render_black(rec, mouse_in_rec); break;
                case Cell::None: break;
                case Cell::Out: break;
            }
        }
    }
    if (mouse_in_any) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

void render_white(Rectangle box, bool mouse_in) {
//     const float padding = 5.0f;
//     Color color = mouse_in ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
//     DrawLineEx(Vector2{.x = box.x + padding, .y = box.y + padding},
//                Vector2{.x = box.x + box.width - padding, .y = box.y - padding + box.height}, 10.0f, color);
//     DrawLineEx(Vector2{.x = box.x + box.width - padding, .y = box.y + padding},
//                Vector2{.x = box.x + padding, .y = box.y - padding + box.height}, 10.0f, color);
    Vector2 mouse = GetMousePosition();
    Color color = mouse_in ? MOUSE_IN_WHITE_COLOR : WHITE_COLOR;
    DrawCircle(box.x + box.width/2.0f, box.y + box.height/2.0f, box.width/2.2f, color);
}

void render_black(Rectangle box, bool mouse_in) {
    Vector2 mouse = GetMousePosition();
    Color color = mouse_in ? MOUSE_IN_BLACK_COLOR : BLACK_COLOR;
    DrawCircle(box.x + box.width/2.0f, box.y + box.height/2.0f, box.width/2.2f, color);
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
