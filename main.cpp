#include <iostream>
#include <raylib.h>
#include <deque>

enum Direction {
    W,
    A,
    S,
    D
};

enum State {
    START,
    PLAY,
    GAMEOVER
};

State game_state = START;


class Snake {
public:
    std::deque<Vector2> dq;

    bool IsInSnake(Vector2 vector) {
        for (Vector2 coordenates : dq) {
            if (vector.x == coordenates.x && vector.y == coordenates.y) {
                return true;
            }
        }
        return false;
    }

    void Move(Direction direction) {

        // Take Front
        Vector2 front = dq.front();
        int index_x = front.x;
        int index_y = front.y;

        if (direction == W) {
            index_y--;
            if (index_y < 0) {
                index_y = 9;
            }
            std::cout << "W" << std::endl;
        }
        else if (direction == A) {
            index_x--;
            if (index_x < 0) {
                index_x = 9;
            }
            std::cout << "A" << std::endl;
        }
        else if (direction == S) {
            index_y++;
            if (index_y > 9) {
                index_y = 0;
            }
            std::cout << "S" << std::endl;
        }
        else if (direction == D) {
            index_x++;
            if (index_x > 9) {
                index_x = 0;
            }
            std::cout << "D" << std::endl;
        }

        // New front
        Vector2 new_front;
        new_front.x = index_x;
        new_front.y = index_y;

        // Check Collision
        if ( IsInSnake(new_front) ) {
            game_state = GAMEOVER;
        }
        // Add new front
        dq.push_front( new_front );

        // Delete Back
        dq.pop_back();

    }

};

class Apple {
public:
    int i, j;

    void Draw() {

    }
};

int main() {

    /*
    SCREEN SETTINGS
    */
    int screen_size = 800;
    InitWindow(screen_size, screen_size, "Snake");
    SetTargetFPS(60);

    /*
    GAME SETTINGS
    */
    float snake_velocity = 0.3f;


    /*
    FLOOR
    */
    int floor_size = screen_size/10;


    /*
    SNAKE
    */
    Direction snake_direction = D;
    Snake snake {std::deque<Vector2>{ Vector2{3,5}, Vector2{4,5}, Vector2{5,5} } };

    float time_count = 0;
    while (!WindowShouldClose()) {

        /*
        CALCULATE
        */
        time_count += GetFrameTime();
        std::cout << "Time: " << time_count << std::endl;




        /*
        UPDATE
        */

        // Move Snake
        if (time_count >= snake_velocity) {
            time_count = 0;
            snake.Move(snake_direction);
        }

        // Change Direction
        if (IsKeyPressed(KEY_W)) {
            snake_direction = W;
        }
        else if (IsKeyPressed(KEY_A)) {
            snake_direction = A;
        }
        else if (IsKeyPressed(KEY_S)) {
            snake_direction = S;
        }
        else if (IsKeyPressed(KEY_D)) {
            snake_direction = D;
        }

        /*
        DRAWING
        */
        BeginDrawing();

        // Floor
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {


                // Floor
                if ( (i + j) % 2 == 0 ) { // It's Pair
                    DrawRectangle(i*screen_size/10, j*screen_size/10, floor_size, floor_size, BLUE);
                }
                else {
                    DrawRectangle(i*screen_size/10, j*screen_size/10, floor_size, floor_size, SKYBLUE);
                }

                // Snake
                Vector2 coordenates;
                coordenates.x = i;
                coordenates.y = j;
                if (snake.dq.front().x == i && snake.dq.front().y == j) { // Head
                    DrawRectangle(i*screen_size/10, j*screen_size/10, floor_size, floor_size, GREEN);

                    DrawRectangle(i*screen_size/10+floor_size/2-30, j*screen_size/10+10, 20, 20, WHITE);
                    DrawRectangle(i*screen_size/10+floor_size/2-30, j*screen_size/10+10, 10, 10, DARKGREEN);

                    DrawRectangle(i*screen_size/10+floor_size/2, j*screen_size/10+10, 20, 20, WHITE);
                    DrawRectangle(i*screen_size/10+floor_size/2, j*screen_size/10+10, 10, 10, DARKGREEN);
                }
                else if ( snake.IsInSnake(coordenates) ) { // Body
                    DrawRectangle(i*screen_size/10, j*screen_size/10, floor_size, floor_size, GREEN);
                }


            };
        }

        EndDrawing();

    }

    CloseWindow();

    return 0;
}
