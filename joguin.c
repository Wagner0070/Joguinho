#include <raylib.h>

int main() {
    InitWindow(800, 600, "Hello World!");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Meu primeiro jogo em C", 190, 280, 20, DARKBLUE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}