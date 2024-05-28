#include "raylib.h"
#include <stdio.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define SNAKE_LENGTH 256
#define SQUARE_SIZE 31

typedef struct Snake {
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    Color color;
} Snake;

typedef struct Food {
    Vector2 position;
    Vector2 size;
    bool active;
    Color color;
} Food;


// variables declaration

static const int screenWidth = 800;
static const int screenHeight = 450;
static const int fieldHeight = 450 - SQUARE_SIZE*2;

static int framesCounter = 0;
static int score = 0;
static int difficulty = 1;
static bool gameOver = false;
static bool pause = false;

static Food fruit = {0};
static Food superFruit = {0};
static Snake snake[SNAKE_LENGTH] = {0}; // snake itself
static Vector2 snakePosition[SNAKE_LENGTH] = {0}; // auxiliary array for snake movement
static bool allowMove = false;
static bool allowSuperFruit = false;
static Vector2 offset = {0};
static int tailLength = 0;


// functions declaration

static void InitGame();         // Initialize game
static void UpdateGame();       // Update game (one frame)
static void DrawGame();         // Draw game (one frame)
static void UpdateDrawFrame();  // Update and Draw (one frame)


int main() {

    InitWindow(screenWidth, screenHeight, "Sssssnake"); 

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(35); // скорость игры
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose()) {

        UpdateDrawFrame();

    }
#endif

    CloseWindow();

    return 0;
}


void InitGame() { // set initial values
    framesCounter = 0;
    gameOver = false;
    pause = false;

    tailLength = 1;
    allowMove = false;
    allowSuperFruit = false;

    difficulty = 1;
    score = 0;

    offset.x = screenWidth % SQUARE_SIZE; // 800 % 31 = 25
    offset.y = fieldHeight % SQUARE_SIZE; // 388 % 31 = 16

    for (int i = 0; i < SNAKE_LENGTH; i++) { 
        snake[i].position = (Vector2){ offset.x/2, offset.y/2 }; // x = 16 / 2 = 8
        snake[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
        snake[i].speed = (Vector2){ SQUARE_SIZE, 0 };

        snakePosition[i] = (Vector2){ 0.0f, 0.0f };

        if (i == 0) snake[i].color = DARKGREEN;
        else snake[i].color = LIME;
    }

    fruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    fruit.color = RED;
    fruit.active = false;

    superFruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    superFruit.color = YELLOW;
    superFruit.active = false;
}


void UpdateGame() {
    if (!gameOver) {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause) {
            // Player control
            if (IsKeyPressed(KEY_RIGHT) && (snake[0].speed.x == 0) && allowMove) {
                snake[0].speed = (Vector2){ SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ -SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, -SQUARE_SIZE };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, SQUARE_SIZE };
                allowMove = false;
            }

            // Snake movement
            for (int i = 0; i < tailLength; i++) snakePosition[i] = snake[i].position;

            if ((framesCounter%5) == 0) {
                for (int i = 0; i < tailLength; i++) {
                    if (i == 0) {
                        snake[0].position.x += snake[0].speed.x;
                        snake[0].position.y += snake[0].speed.y;
                        allowMove = true;
                    }
                    else snake[i].position = snakePosition[i-1];
                }
            }

            // Colision with walls
            switch (difficulty) {
            case 1:
                if (snake[0].position.x < 0) {
                    snake[0].position.x = screenWidth - offset.x / 2 - SQUARE_SIZE;
                } 
                else if ((snake[0].position.x) > (screenWidth - offset.x)) {
                    snake[0].position.x = offset.x / 2;
                }
                
                if (snake[0].position.y < 0) {
                    snake[0].position.y = fieldHeight - offset.y / 2 - SQUARE_SIZE;
                }
                else if ((snake[0].position.y) > (fieldHeight - offset.y)) {
                    snake[0].position.y = offset.y / 2;
                } 
                break;
            case 2: // only side walls are dangerous
                if (((snake[0].position.x) > (screenWidth - offset.x)) ||
                    (snake[0].position.x < 0)) {
                    gameOver = true;
                }

                if (snake[0].position.y < 0) {
                    snake[0].position.y = fieldHeight - offset.y / 2 - SQUARE_SIZE;
                }
                else if ((snake[0].position.y) > (fieldHeight - offset.y)) {
                    snake[0].position.y = offset.y / 2;
                } 
                
                break;
            case 3: // all four walls are dangerous
                if (((snake[0].position.x) > (screenWidth - offset.x)) ||
                   ((snake[0].position.y) > (fieldHeight - offset.y)) ||
                   (snake[0].position.x < 0) || (snake[0].position.y < 0)) {
                    gameOver = true;
                }
                break;
            }
            

            // Collision with yourself
            for (int i = 1; i < tailLength; i++) {
                if ((snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y)) gameOver = true;
            }

            // Fruit position calculation
            if (!fruit.active) {
                fruit.active = true;
                fruit.position = (Vector2){GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };

                for (int i = 0; i < tailLength; i++) { // if the fruit generates on the snake
                    while ((fruit.position.x == snake[i].position.x) && (fruit.position.y == snake[i].position.y)) {
                        fruit.position = (Vector2){ GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };
                        i = 0;
                    }
                }
            }

            // Collision with the fruit (when head is on it already)
            if ((snake[0].position.x < (fruit.position.x + fruit.size.x) && (snake[0].position.x + snake[0].size.x) > fruit.position.x) &&
                (snake[0].position.y < (fruit.position.y + fruit.size.y) && (snake[0].position.y + snake[0].size.y) > fruit.position.y))
            {
                snake[tailLength].position = snakePosition[tailLength - 1];
                tailLength += 1;
                score += 1;
                fruit.active = false;
            }

            // Same for superFruit
            // superFruit position calculation
            //
            if((difficulty >= 2) && ((framesCounter / 35) % 8 == 0)) allowSuperFruit = true;
            if (!superFruit.active && allowSuperFruit) {
                superFruit.active = true;
                superFruit.position = (Vector2){GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };

                for (int i = 0; i < tailLength; i++) { // if the superFruit generates on the snake
                    while ((superFruit.position.x == snake[i].position.x) && (superFruit.position.y == snake[i].position.y)) {
                        superFruit.position = (Vector2){ GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };
                        i = 0;
                    }
                }
                // if superFruit generates on usual fruit
                while ((fruit.position.x == superFruit.position.x) && (fruit.position.y == superFruit.position.y)) {
                        superFruit.position = (Vector2){ GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };
                    }
            }

            // Collision with the superFruit (when head is on it already)
            if ((snake[0].position.x < (superFruit.position.x + superFruit.size.x) && (snake[0].position.x + snake[0].size.x) > superFruit.position.x) &&
                (snake[0].position.y < (superFruit.position.y + superFruit.size.y) && (snake[0].position.y + snake[0].size.y) > superFruit.position.y))
            {
                // for(int j = 1; j <= 2 ; j++) {
                //     snake[tailLength].position = snakePosition[tailLength - 1];
                //     snakePosition[tailLength] = snake[tailLength].position;
                //     tailLength += 1;
                // }
                snake[tailLength].position = snakePosition[tailLength - 1];
                //snakePosition[tailLength] = snake[tailLength].position;
                tailLength += 1;
                //snake[tailLength].position = snakePosition[tailLength - 1];
                //snakePosition[tailLength] = snake[tailLength].position;
                //tailLength += 1;

                score += 2;
                superFruit.active = false;
                allowSuperFruit = false;
            }

            if(score == 6) {
                difficulty = 2;
            } else if (score >= 12) {
                difficulty = 3;
            }

            framesCounter++;
        }
    } else {
            if (IsKeyPressed(KEY_ENTER)) {
                InitGame();
                gameOver = false;
            }
        }
}

void DrawGame() {
    
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if(!gameOver) {
        // Draw field lines:
        for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++) {
            DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE*i + offset.x/2, fieldHeight - offset.y/2}, LIGHTGRAY);
        }

        for (int i = 0; i < fieldHeight/SQUARE_SIZE + 1; i++) {
            DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, (Vector2){screenWidth - offset.x/2, SQUARE_SIZE*i + offset.y/2}, LIGHTGRAY);
        }

        // Draw snake:
        for (int i = 0; i < tailLength; i++) DrawRectangleV(snake[i].position, snake[i].size, snake[i].color);

        // Draw fruit
        DrawRectangleV(fruit.position, fruit.size, fruit.color);

        if(superFruit.active) DrawRectangleV(superFruit.position, superFruit.size, superFruit.color);

        if (pause) DrawText("PAUSED", screenWidth/2 - MeasureText("PAUSED", 50)/2, fieldHeight/2 - 40, 50, RED);

        // draw score:
        DrawText("SCORE: ", offset.x/2, screenHeight - SQUARE_SIZE - 25, 28, RED);
        char scoreStr[256];
        sprintf(scoreStr, "%d", score);
        DrawText(scoreStr, offset.x/2 + SQUARE_SIZE * 4, screenHeight - SQUARE_SIZE - 25, 28, RED);

        // draw difficulty:
        DrawText("DIFFICULTY: ", screenWidth - offset.x/2 -  MeasureText("DIFFICULTY: ", 28) - 30, screenHeight - SQUARE_SIZE - 25, 28, RED);
        char diffStr[256];
        sprintf(diffStr, "%d", difficulty);
        DrawText(diffStr, screenWidth - offset.x/2 - 30, screenHeight - SQUARE_SIZE - 25, 28, RED);

        char timeStr[256];
        float time2 = framesCounter;
        sprintf(timeStr, "%d", tailLength);
        DrawText(timeStr, screenWidth/2 - MeasureText(timeStr, 28)/2, screenHeight - SQUARE_SIZE - 25, 28, RED);
    }

    else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 30)/2, fieldHeight/2 - 50, 30, RED);

    EndDrawing();
}

void UpdateDrawFrame() {
    UpdateGame();
    DrawGame();
}