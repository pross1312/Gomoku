#include "Game.h"
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
    InitWindow(1300, 800, "Gomoku");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(100, 100);
    SetTargetFPS(30);

    Game game;

    game.run();

    CloseWindow();
    return 0;
}
