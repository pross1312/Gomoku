#pragma once
#include "BitBoard.h"
#include <raylib.h>

#define CELL_SIZE 50
#define PADDING 5
#define EMPTY_COLOR GetColor(0xcc8c3cff) // LIGHTGRAY
#define MOUSE_IN_EMPTY_COLOR GetColor(0xee9c3cff)
#define WHITE_COLOR WHITE
#define MOUSE_IN_WHITE_COLOR WHITE
#define BLACK_COLOR BLACK
#define MOUSE_IN_BLACK_COLOR BLACK

void render_board(const BitBoard &board, const Rectangle rec);
void render_white(Rectangle box, bool mouse_in);
void render_black(Rectangle box, bool mouse_in);
void render_line(uint32_t line);
