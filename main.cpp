#include <iostream>
#include <raylib.h>
#include <deque>

constexpr int GRID_SIZE = 10;
constexpr int SCREEN_SIZE = 800;
constexpr int CELL_SIZE = SCREEN_SIZE / GRID_SIZE;

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
enum State { PLAY, GAMEOVER };

class Apple {
public:
    Vector2 pos;

    Apple() { pos = {5, 8}; }

    void Respawn(const std::deque<Vector2>& snakeBody) {
        bool onSnake = true;
        while (onSnake) {
            pos.x = GetRandomValue(0, GRID_SIZE - 1);
            pos.y = GetRandomValue(0, GRID_SIZE - 1);
            onSnake = false;
            for (const auto& segment : snakeBody) {
                if (segment.x == pos.x && segment.y == pos.y) {
                    onSnake = true;
                    break;
                }
            }
        }
    }
};

class Snake {
public:
    std::deque<Vector2> body;

    Snake() {
        body = { {5, 5}, {4, 5}, {3, 5} };
    }

    bool IsInSnake(const Vector2& vec, bool ignoreTail = false) const {
        size_t limit = ignoreTail ? body.size() - 1 : body.size();
        for (size_t i = 0; i < limit; ++i) {
            if (vec.x == body[i].x && vec.y == body[i].y) {
                return true;
            }
        }
        return false;
    }

    void Move(Direction dir, Apple& apple, State& gameState) {
        if (dir == NONE) return;

        Vector2 newHead = body.front();

        switch (dir) {
            case UP:    newHead.y = (newHead.y - 1 < 0) ? GRID_SIZE - 1 : newHead.y - 1; break;
            case DOWN:  newHead.y = (newHead.y + 1 >= GRID_SIZE) ? 0 : newHead.y + 1; break;
            case LEFT:  newHead.x = (newHead.x - 1 < 0) ? GRID_SIZE - 1 : newHead.x - 1; break;
            case RIGHT: newHead.x = (newHead.x + 1 >= GRID_SIZE) ? 0 : newHead.x + 1; break;
            default: break;
        }

        // Si colisiona con el cuerpo (ignorando la cola que va a avanzar)
        bool eatsApple = (newHead.x == apple.pos.x && newHead.y == apple.pos.y);
        if (IsInSnake(newHead, !eatsApple)) {
            gameState = GAMEOVER;
            return;
        }

        body.push_front(newHead);

        if (eatsApple) {
            apple.Respawn(body);
        } else {
            body.pop_back();
        }
    }

    void Reset() {
        body = { {5, 5}, {4, 5}, {3, 5} };
    }
};

int main() {
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Snake - Optimizado");
    SetTargetFPS(60);

    State gameState = PLAY;
    float snakeSpeed = 0.15f;
    float timer = 0.0f;

    Direction inputDir = RIGHT;
    Direction currentDir = RIGHT;

    Snake snake;
    Apple apple;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. INPUT
        if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && currentDir != DOWN)    inputDir = UP;
        if ((IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) && currentDir != UP)    inputDir = DOWN;
        if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && currentDir != RIGHT) inputDir = LEFT;
        if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && currentDir != LEFT) inputDir = RIGHT;

        if (gameState == GAMEOVER && IsKeyPressed(KEY_R)) {
            snake.Reset();
            apple.Respawn(snake.body);
            inputDir = RIGHT;
            currentDir = RIGHT;
            gameState = PLAY;
        }

        // 2. UPDATE
        if (gameState == PLAY) {
            timer += dt;
            if (timer >= snakeSpeed) {
                timer = 0.0f;
                currentDir = inputDir;
                snake.Move(currentDir, apple, gameState);
            }
        }

        // 3. DRAW
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Fondo de tablero
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                DrawRectangle(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                              ((i + j) % 2 == 0) ? BLUE : SKYBLUE);
            }
        }

        // Manzana
        DrawRectangle(apple.pos.x * CELL_SIZE + 15, apple.pos.y * CELL_SIZE + 15, 50, 50, RED);
        DrawRectangle(apple.pos.x * CELL_SIZE + 35, apple.pos.y * CELL_SIZE + 5, 10, 18, BROWN);

        // Snake
        for (size_t i = 0; i < snake.body.size(); ++i) {
            int px = snake.body[i].x * CELL_SIZE;
            int py = snake.body[i].y * CELL_SIZE;

            DrawRectangle(px, py, CELL_SIZE, CELL_SIZE, GREEN);

            // Eyes
            if (i == 0) {
                DrawRectangle(px + 15, py + 15, 15, 15, WHITE);
                DrawRectangle(px + 18, py + 18, 8, 8, BLACK);
                DrawRectangle(px + 50, py + 15, 15, 15, WHITE);
                DrawRectangle(px + 53, py + 18, 8, 8, BLACK);
            }
        }

        // Game Over
        if (gameState == GAMEOVER) {
            DrawRectangle(0, 0, SCREEN_SIZE, SCREEN_SIZE, Fade(BLACK, 0.6f));
            DrawText("GAME OVER", SCREEN_SIZE / 2 - MeasureText("GAME OVER", 60) / 2, 320, 60, RED);
            DrawText("Presiona [R] para reiniciar", SCREEN_SIZE / 2 - MeasureText("Presiona [R] para reiniciar", 20) / 2, 410, 20, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}