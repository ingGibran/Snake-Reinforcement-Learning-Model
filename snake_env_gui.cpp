#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Permite convertir std::vector de C++ a listas de Python
#include <raylib.h>       // <-- LIBRERÍA GRÁFICA AÑADIDA
#include <deque>
#include <vector>
#include <random>

namespace py = pybind11;

constexpr int GRID_SIZE = 10;
constexpr int SCREEN_SIZE = 800;                   // <-- AÑADIDO
constexpr int CELL_SIZE = SCREEN_SIZE / GRID_SIZE; // <-- AÑADIDO

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
enum Action { STRAIGHT = 0, TURN_RIGHT = 1, TURN_LEFT = 2 };

struct Point { int x, y; };

class Apple {
public:
    Point pos;
    std::mt19937 rng;

    Apple() {
        std::random_device rd;
        rng.seed(rd());
        pos = {5, 8};
    }

    void Respawn(const std::deque<Point>& snakeBody) {
        std::uniform_int_distribution<int> dist(0, GRID_SIZE - 1);
        bool onSnake = true;
        while (onSnake) {
            pos.x = dist(rng);
            pos.y = dist(rng);
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
    std::deque<Point> body;
    Snake() { Reset(); }

    bool IsInSnake(const Point& pt, bool ignoreTail = false) const {
        size_t limit = ignoreTail ? body.size() - 1 : body.size();
        for (size_t i = 0; i < limit; ++i) {
            if (pt.x == body[i].x && pt.y == body[i].y) return true;
        }
        return false;
    }
    void Reset() { body = { {5, 5}, {4, 5}, {3, 5} }; }
};

struct StepResult {
    std::vector<int> state;
    float reward;
    bool done;
    int score;
};

class SnakeEnv {
public:
    Snake snake;
    Apple apple;
    Direction currentDir;
    int score;

    SnakeEnv() { Reset(); }

    void Reset() {
        snake.Reset();
        apple.Respawn(snake.body);
        currentDir = RIGHT;
        score = 0;
    }

    Point GetNextPoint(Point head, Direction dir) const {
        Point next = head;
        if (dir == UP)    next.y -= 1;
        if (dir == DOWN)  next.y += 1;
        if (dir == LEFT)  next.x -= 1;
        if (dir == RIGHT) next.x += 1;
        return next;
    }

    bool IsDanger(Point pt) const {
        if (pt.x < 0 || pt.x >= GRID_SIZE || pt.y < 0 || pt.y >= GRID_SIZE) return true;
        return snake.IsInSnake(pt);
    }

    std::vector<int> GetState() {
        Point head = snake.body.front();

        Direction dir_l = currentDir == UP ? LEFT : currentDir == DOWN ? RIGHT : currentDir == LEFT ? DOWN : UP;
        Direction dir_r = currentDir == UP ? RIGHT : currentDir == DOWN ? LEFT : currentDir == LEFT ? UP : DOWN;

        Point pt_straight = GetNextPoint(head, currentDir);
        Point pt_right = GetNextPoint(head, dir_r);
        Point pt_left = GetNextPoint(head, dir_l);

        std::vector<int> state = {
            IsDanger(pt_straight) ? 1 : 0, IsDanger(pt_right) ? 1 : 0, IsDanger(pt_left) ? 1 : 0,
            currentDir == LEFT ? 1 : 0, currentDir == RIGHT ? 1 : 0, currentDir == UP ? 1 : 0, currentDir == DOWN ? 1 : 0,
            apple.pos.x < head.x ? 1 : 0, apple.pos.x > head.x ? 1 : 0, apple.pos.y < head.y ? 1 : 0, apple.pos.y > head.y ? 1 : 0
        };
        return state;
    }

    StepResult Step(Action action) {
        float reward = 0.0f;
        bool done = false;

        Direction clockwise[4] = { RIGHT, DOWN, LEFT, UP };
        int idx = 0;
        for(int i=0; i<4; i++) if(clockwise[i] == currentDir) idx = i;

        if (action == TURN_RIGHT) currentDir = clockwise[(idx + 1) % 4];
        else if (action == TURN_LEFT) currentDir = clockwise[(idx + 3) % 4];

        Point newHead = GetNextPoint(snake.body.front(), currentDir);
        bool eatsApple = (newHead.x == apple.pos.x && newHead.y == apple.pos.y);
        
        if (IsDanger(newHead) && (!eatsApple || snake.IsInSnake(newHead, true))) {
            done = true;
            reward = -10.0f;
            return { GetState(), reward, done, score };
        }

        snake.body.push_front(newHead);

        if (eatsApple) {
            score++;
            reward = 10.0f;
            apple.Respawn(snake.body);
        } else {
            snake.body.pop_back();
        }

        return { GetState(), reward, done, score };
    }

    // --- NUEVO MÓDULO RENDER ---
    void Render(int n_games, int record, float epsilon) {
        if (!IsWindowReady()) {
            InitWindow(SCREEN_SIZE, SCREEN_SIZE, "IA Jugando - Snake RL");
            SetTargetFPS(120); 
        }

        WindowShouldClose(); // Vital para que el OS no bloquee la ventana

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Tablero
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                DrawRectangle(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE, 
                              ((i + j) % 2 == 0) ? BLUE : SKYBLUE);
            }
        }

        // Manzana
        DrawRectangle(apple.pos.x * CELL_SIZE + 15, apple.pos.y * CELL_SIZE + 15, 50, 50, RED);
        DrawRectangle(apple.pos.x * CELL_SIZE + 35, apple.pos.y * CELL_SIZE + 5, 10, 18, BROWN);

        // Serpiente
        for (size_t i = 0; i < snake.body.size(); ++i) {
            int px = snake.body[i].x * CELL_SIZE;
            int py = snake.body[i].y * CELL_SIZE;

            DrawRectangle(px, py, CELL_SIZE, CELL_SIZE, GREEN);

            // Detalles de la cabeza
            if (i == 0) {
                DrawRectangle(px + 15, py + 15, 15, 15, WHITE);
                DrawRectangle(px + 18, py + 18, 8, 8, BLACK);
                DrawRectangle(px + 50, py + 15, 15, 15, WHITE);
                DrawRectangle(px + 53, py + 18, 8, 8, BLACK);
            }
        }

        // HUD (Barra superior)
        DrawRectangle(0, 0, SCREEN_SIZE, 40, Fade(BLACK, 0.7f));
        DrawText(TextFormat("Partida: %i", n_games), 20, 10, 20, RAYWHITE);
        DrawText(TextFormat("Puntos: %i", score), 200, 10, 20, RAYWHITE);
        DrawText(TextFormat("Record: %i", record), 400, 10, 20, GREEN);
        DrawText(TextFormat("Epsilon: %.1f", epsilon), 600, 10, 20, YELLOW);

        EndDrawing();
    }
};

// --- BINDINGS DE PYBIND11 ---
PYBIND11_MODULE(snake_env_gui, m) {
    m.doc() = "Entorno de Snake en C++ para Reinforcement Learning";

    py::enum_<Action>(m, "Action")
        .value("STRAIGHT", Action::STRAIGHT)
        .value("TURN_RIGHT", Action::TURN_RIGHT)
        .value("TURN_LEFT", Action::TURN_LEFT)
        .export_values();

    py::class_<StepResult>(m, "StepResult")
        .def_readonly("state", &StepResult::state)
        .def_readonly("reward", &StepResult::reward)
        .def_readonly("done", &StepResult::done)
        .def_readonly("score", &StepResult::score);

    py::class_<SnakeEnv>(m, "SnakeEnv")
        .def(py::init<>())
        .def("Reset", &SnakeEnv::Reset)
        .def("Step", &SnakeEnv::Step)
        .def("GetState", &SnakeEnv::GetState)
        .def("Render", &SnakeEnv::Render); // <-- FUNCIÓN EXPORTADA
}