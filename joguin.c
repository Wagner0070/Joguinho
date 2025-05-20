#include <raylib.h>

#define MAX_SHOTS 50
#define MAX_ENEMIES 10
#define MAX_LIVES 3

typedef struct {
    Vector2 pos;
    bool active;
} Shot;

typedef struct {
    Vector2 pos;
    bool active;
} Enemy;

// Função para centralizar texto
void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
    int width = MeasureText(text, fontSize);
    DrawText(text, (GetScreenWidth() - width) / 2, y, fontSize, color);
}

int main(void) {
    int screenWidth = 800;
    int screenHeight = 600;
    bool fullscreen = false;

    InitWindow(screenWidth, screenHeight, "Spacian paper");
    SetTargetFPS(60);

    // Carrega texturas
    Texture2D playerTex = LoadTexture("sprites/Aviãozin.png");
    Texture2D shotTex = LoadTexture("sprites/pencil_ballet.png");
    Texture2D enemyTex = LoadTexture("sprites/bad_alien.png");

    // Frame info
    const int frameWidth = 64;
    const int frameHeight = 64;
    const int maxFrames = 8;
    int currentFrame = 0;
    int frameCounter = 0;

    // Posição inicial do avião (centralizado embaixo)
    Vector2 position = { (float)(screenWidth - frameWidth)/2, (float)(screenHeight - frameHeight - 10) };
    float speed = 4.0f;
    Rectangle frameRec = { 0.0f, 0.0f, frameWidth, frameHeight };
    int direction = 1;

    Shot shots[MAX_SHOTS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};
    float enemySpawnTimer = 0;

    // Delay após perder vida
    const float lifeLostDelay = 1.5f;
    float lifeLostTimer = 0.0f;
    bool waitingAfterLifeLost = false;

    int score = 0;
    int lives = MAX_LIVES;
    bool gameOver = false;

    while (!WindowShouldClose()) {
        // Alternar tela cheia/janela
        if (IsKeyPressed(KEY_F)) {
            fullscreen = !fullscreen;
            if (fullscreen) {
                SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
                ToggleFullscreen();
            } else {
                if (IsWindowFullscreen()) ToggleFullscreen();
                SetWindowSize(screenWidth, screenHeight);
            }
        }

        if (!gameOver) {
            // Delay após perder vida
            if (waitingAfterLifeLost) {
                lifeLostTimer += GetFrameTime();
                if (lifeLostTimer >= lifeLostDelay) {
                    waitingAfterLifeLost = false;
                    lifeLostTimer = 0.0f;
                }
            } else {
                // ----- INPUT PLAYER -----
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                    position.x += speed;
                    direction = 1;
                }
                else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                    position.x -= speed;
                    direction = -1;
                }

                // Tela infinita horizontal
                if (position.x < -frameWidth) position.x = GetScreenWidth();
                if (position.x > GetScreenWidth()) position.x = -frameWidth;

                // ----- DISPARO -----
                if (IsKeyPressed(KEY_SPACE)) {
                    for (int i = 0; i < MAX_SHOTS; i++) {
                        if (!shots[i].active) {
                            int shotWidth = shotTex.width;
                            shots[i].pos = (Vector2){ position.x + (frameWidth - shotWidth)/2, position.y };
                            shots[i].active = true;
                            break;
                        }
                    }
                }

                // ----- ATUALIZA PROJÉTEIS -----
                for (int i = 0; i < MAX_SHOTS; i++) {
                    if (shots[i].active) {
                        shots[i].pos.y -= 8;
                        if (shots[i].pos.y < -16) shots[i].active = false;
                    }
                }

                // ----- SPAWN INIMIGOS -----
                enemySpawnTimer += GetFrameTime();
                if (enemySpawnTimer > 1.0f) {
                    enemySpawnTimer = 0;
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (!enemies[i].active) {
                            enemies[i].pos = (Vector2){ GetRandomValue(0, GetScreenWidth()-enemyTex.width), -enemyTex.height };
                            enemies[i].active = true;
                            break;
                        }
                    }
                }

                // ----- ATUALIZA INIMIGOS -----
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        enemies[i].pos.y += 2;
                        // Se inimigo passar do final da tela, perde vida
                        if (enemies[i].pos.y > GetScreenHeight()) {
                            enemies[i].active = false;
                            lives--;
                            if (lives <= 0) gameOver = true;
                            else {
                                waitingAfterLifeLost = true;
                                lifeLostTimer = 0.0f;
                                for (int j = 0; j < MAX_ENEMIES; j++) enemies[j].active = false;
                            }
                        }
                    }
                }

                // ----- COLISÃO TIRO x INIMIGO -----
                for (int i = 0; i < MAX_SHOTS; i++) {
                    if (shots[i].active) {
                        Rectangle shotRect = { shots[i].pos.x, shots[i].pos.y, shotTex.width, shotTex.height };
                        for (int j = 0; j < MAX_ENEMIES; j++) {
                            if (enemies[j].active) {
                                Rectangle enemyRect = { enemies[j].pos.x, enemies[j].pos.y, enemyTex.width, enemyTex.height };
                                if (CheckCollisionRecs(shotRect, enemyRect)) {
                                    shots[i].active = false;
                                    enemies[j].active = false;
                                    score += 10;
                                    break;
                                }
                            }
                        }
                    }
                }

                // ----- COLISÃO INIMIGO x AVIÃO -----
                Rectangle playerRect = { position.x, position.y, frameWidth, frameHeight };
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        Rectangle enemyRect = { enemies[i].pos.x, enemies[i].pos.y, enemyTex.width, enemyTex.height };
                        if (CheckCollisionRecs(playerRect, enemyRect)) {
                            enemies[i].active = false;
                            lives--;
                            if (lives <= 0) gameOver = true;
                            else {
                                waitingAfterLifeLost = true;
                                lifeLostTimer = 0.0f;
                                for (int j = 0; j < MAX_ENEMIES; j++) enemies[j].active = false;
                            }
                        }
                    }
                }

                // ----- ANIMAÇÃO -----
                frameCounter++;
                if (frameCounter >= 8) {
                    frameCounter = 0;
                    currentFrame++;
                    if (currentFrame >= maxFrames) currentFrame = 0;
                    frameRec.x = (float)currentFrame * frameWidth;
                }
            }
        } else {
            // Reiniciar o jogo ao pressionar R
            if (IsKeyPressed(KEY_R)) {
                score = 0;
                lives = MAX_LIVES;
                gameOver = false;
                for (int i = 0; i < MAX_SHOTS; i++) shots[i].active = false;
                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
                position.x = (float)(GetScreenWidth() - frameWidth)/2;
                position.y = (float)(GetScreenHeight() - frameHeight - 10);
                waitingAfterLifeLost = false;
                lifeLostTimer = 0.0f;
            }
        }

        // ----- RENDER -----
        BeginDrawing();
            ClearBackground(BLACK);

            // Player
            if (!gameOver) {
                if (direction == -1) {
                    DrawTextureRec(playerTex,
                        (Rectangle){ frameRec.x + frameWidth, frameRec.y, -frameWidth, frameHeight },
                        position, WHITE);
                } else {
                    DrawTextureRec(playerTex, frameRec, position, WHITE);
                }
            }

            // Projéteis
            for (int i = 0; i < MAX_SHOTS; i++) {
                if (shots[i].active)
                    DrawTexture(shotTex, (int)shots[i].pos.x, (int)shots[i].pos.y, WHITE);
            }

            // Inimigos (só desenha se não está no delay)
            if (!waitingAfterLifeLost) {
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active)
                        DrawTexture(enemyTex, (int)enemies[i].pos.x, (int)enemies[i].pos.y, WHITE);
                }
            }

            // Pontuação e vidas
            DrawText(TextFormat("Pontuação: %d", score), 10, 10, 24, YELLOW);
            DrawText(TextFormat("Vidas: %d", lives), 10, 40, 24, RED);

            // FPS de performance
            DrawFPS(GetScreenWidth() - 100, 10);

            // Instruções e mensagens
            if (!gameOver) {
                DrawText("A/D ou ←/→ para mover | Espaço para atirar | F para tela cheia", 10, 70, 18, RAYWHITE);
                if (waitingAfterLifeLost) {
                    DrawCenteredText("Prepare-se!", GetScreenHeight()/2 - 18, 36, ORANGE);
                }
            } else {
                DrawCenteredText("GAME OVER", GetScreenHeight()/2 - 48, 48, RED);
                DrawCenteredText("Pressione R para reiniciar", GetScreenHeight()/2 + 20, 28, YELLOW);
            }
        EndDrawing();
    }

    UnloadTexture(playerTex);
    UnloadTexture(shotTex);
    UnloadTexture(enemyTex);
    CloseWindow();

    return 0;
}