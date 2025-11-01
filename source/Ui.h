#pragma once
#include "BitBoard.h"
#include <optional>
#include <raylib.h>

#define CELL_SIZE 50
#define EMPTY_COLOR GetColor(0xcc8c3cff) // LIGHTGRAY
#define MOUSE_IN_EMPTY_COLOR GetColor(0xee9c3cff)
#define WHITE_COLOR WHITE
#define MOUSE_IN_WHITE_COLOR WHITE
#define BLACK_COLOR BLACK
#define LAST_MOVE_COLOR GetColor(0x48963bff)
#define OVERLAY_COLOR GetColor(0x0000008f)
#define MOUSE_IN_BLACK_COLOR BLACK
#define TEXT_HIGHLIGHT_COLOR YELLOW
#define TEXT_COLOR WHITE

struct Ui {
private:
    Rectangle bound;
    Rectangle board_bound;
    float square_size;
    float padding;

public:
    float font_size = 30;

    Ui() = default;
    Ui(Rectangle bound) { set_bound(bound); }

    void set_bound(Rectangle new_bound);

    void render_win(const BitBoard &board, Figure turn);
    void render_draw(const BitBoard &board);
    void render_overlay_with_text(const char* text);
    void render_board(const BitBoard &board);
    void render_white(Rectangle box, bool mouse_in);
    void render_black(Rectangle box, bool mouse_in);
    void render_line(uint32_t line);

    Coord get_cell_at_pos(Vector2 pos);
};
