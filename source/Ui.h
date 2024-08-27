#pragma once
#include "BitBoard.h"
#include <raylib.h>

#define CELL_SIZE 50
#define PADDING 5
#define EMPTY_COLOR LIGHTGRAY
#define MOUSE_IN_EMPTY_COLOR GRAY
#define WHITE_COLOR BLUE
#define MOUSE_IN_WHITE_COLOR SKYBLUE
#define BLACK_COLOR RED
#define MOUSE_IN_BLACK_COLOR PINK

bool render_board(const BitBoard &board);
void render_white(Rectangle box, bool mouse_in);
void render_black(Rectangle box, bool mouse_in);
void render_line(uint32_t line);
