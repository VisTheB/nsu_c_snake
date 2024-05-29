#include "raylib.h"
#include <stdio.h>

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
static double time;
static bool gameOver = false;
static bool pause = false;

static Food fruit = {0};
static Food superFruit = {0};
static Snake snake[SNAKE_LENGTH] = {0}; // snake itself
static Vector2 snakePosition[SNAKE_LENGTH] = {0}; // auxiliary array for snake movement
static bool allowMove = false;
static bool allowFruit = true;
static bool allowSuperFruit = false;
static Vector2 offset = {0};
static int tailLength = 0;


// functions declaration

static void InitGame();         // Initialize game
static void UpdateGame();       // Update game (one frame)
static void DrawGame();         // Draw game (one frame)
static void UpdateDrawFrame();  // Update and Draw (one frame)
static void manageFruit(Food *fruit, bool *allowSuperFruit2, bool isSuperFruit); // updates fruit variables


int main() {

    InitWindow(screenWidth, screenHeight, "Sssssnake"); 

    InitAudioDevice();
    Music music = LoadMusicStream("Crazy_Frog.mp3");
    music.looping = false;

    InitGame();
    PlayMusicStream(music);

    SetTargetFPS(35); // скорость игры
    
    while (!WindowShouldClose()) {

        UpdateMusicStream(music); 
        UpdateDrawFrame();

    }

    UnloadMusicStream(music);
    CloseAudioDevice();

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
    allowFruit = true;

    difficulty = 1;
    score = 0;
    time = 0;

    offset.x = screenWidth % SQUARE_SIZE; // 800 % 31 = 25
    offset.y = fieldHeight % SQUARE_SIZE; // 388 % 31 = 16

    for (int i = 0; i < SNAKE_LENGTH; i++) { 
        snake[i].position = (Vector2){ offset.x/2, offset.y/2 }; // y = 16 / 2 = 8
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
    superFruit.color = GOLD;
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
                if (((snake[0].position.x) > (screenWidth - offset.x - SQUARE_SIZE)) ||
                    (snake[0].position.x < SQUARE_SIZE)) {
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
                if (((snake[0].position.x) > (screenWidth - offset.x - SQUARE_SIZE)) ||
                   ((snake[0].position.y) > (fieldHeight - offset.y - SQUARE_SIZE)) ||
                   (snake[0].position.x < SQUARE_SIZE) || (snake[0].position.y < SQUARE_SIZE)) {
                    gameOver = true;
                }
                break;
            }
            

            // Collision with yourself
            for (int i = 1; i < tailLength; i++) {
                if ((snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y)) gameOver = true;
            }

            // fruit position calculation
            manageFruit(&fruit, &allowFruit, false);
            allowFruit = true;

            // Same for superFruit
            // superFruit position calculation
            if(difficulty >= 2) allowSuperFruit = true;
            manageFruit(&superFruit, &allowSuperFruit, true);

            // difficulty change
            if(score == 7) {
                difficulty = 2;
            } else if (score >= 15) {
                difficulty = 3;
            }

            framesCounter++;
        }
    } else {
            if(IsKeyPressed(KEY_ENTER)) {
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
        switch (difficulty) {
        case 2:
            DrawRectangleV((Vector2){offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE, fieldHeight - offset.y}, MAROON);
            DrawRectangleV((Vector2){screenWidth - offset.x / 2 - SQUARE_SIZE, offset.y/2}, (Vector2){SQUARE_SIZE, fieldHeight - offset.y}, MAROON);
            break;
        case 3:
            DrawRectangleV((Vector2){offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE, fieldHeight - offset.y}, MAROON);
            DrawRectangleV((Vector2){screenWidth - offset.x/2 - SQUARE_SIZE, offset.y/2}, (Vector2){SQUARE_SIZE, fieldHeight - offset.y}, MAROON);

            DrawRectangleV((Vector2){offset.x/2, offset.y/2}, (Vector2){screenWidth - offset.x, SQUARE_SIZE}, MAROON);
            DrawRectangleV((Vector2){offset.x/2, fieldHeight - offset.y/2 - SQUARE_SIZE}, (Vector2){screenWidth - offset.x, SQUARE_SIZE}, MAROON);            
            break;
        }
        

        // Draw snake:
        for (int i = 0; i < tailLength; i++) DrawRectangleV(snake[i].position, snake[i].size, snake[i].color);

        // Draw fruit
        DrawRectangleV(fruit.position, fruit.size, fruit.color);
        // Draw superFruit
        if(superFruit.active) DrawRectangleV(superFruit.position, superFruit.size, superFruit.color);

        if (pause) DrawText("PAUSED", screenWidth/2 - MeasureText("PAUSED", 50)/2, fieldHeight/2 - 40, 50, RED);

        // draw score:
        DrawText("SCORE: ", offset.x/2, screenHeight - SQUARE_SIZE - 25, 28, VIOLET);
        char scoreStr[256];
        sprintf(scoreStr, "%d", score);
        DrawText(scoreStr, offset.x/2 + SQUARE_SIZE * 4, screenHeight - SQUARE_SIZE - 25, 28, VIOLET);

        // draw difficulty:
        DrawText("DIFFICULTY: ", screenWidth - offset.x/2 -  MeasureText("DIFFICULTY: ", 28) - 30, screenHeight - SQUARE_SIZE - 25, 28, VIOLET);
        char diffStr[256];
        sprintf(diffStr, "%d", difficulty);
        DrawText(diffStr, screenWidth - offset.x/2 - 30, screenHeight - SQUARE_SIZE - 25, 28, VIOLET);

        char timeStr[256];
        time = GetTime();
        sprintf(timeStr, "%d", (int)time);
        DrawText(timeStr, screenWidth/2 - MeasureText(timeStr, 28)/2, screenHeight - SQUARE_SIZE - 25, 28, VIOLET);
    }

    else {
        DrawText("LOOSER, [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("LOOSER, [ENTER] TO PLAY AGAIN", 30)/2, fieldHeight/2 - 50, 30, VIOLET);
    }

    EndDrawing();
}

void UpdateDrawFrame() {
    UpdateGame();
    DrawGame();
}

void manageFruit(Food *fruit2, bool *allowSuperFruit2, bool isSuperFruit) {

    // Fruit position calculation
    if (!fruit2->active && (*allowSuperFruit2)) {
        fruit2->active = true;
        switch (difficulty) {
        case 1:
            fruit2->position = (Vector2){GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2};
            break;
        
        case 2:
            fruit2->position = (Vector2){GetRandomValue(1, (screenWidth/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (fieldHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2};                    
            break;

        case 3:
            fruit2->position = (Vector2){GetRandomValue(1, (screenWidth/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.x/2, GetRandomValue(1, (fieldHeight/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.y/2};                    
            break;
        }

        for (int i = 0; i < tailLength; i++) { // if the fruit generates on the snake
            while ((fruit2->position.x == snake[i].position.x) && (fruit2->position.y == snake[i].position.y)) {
                fruit2->position = (Vector2){ GetRandomValue(1, (screenWidth/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.x/2, GetRandomValue(1, (fieldHeight/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.y/2 };
                i = 0;
            }
        }

        // if superFruit generates on usual fruit или наоборот
        if(isSuperFruit) {
            while ((fruit.position.x == fruit2->position.x) && (fruit.position.y == fruit2->position.y)) {
                fruit2->position = (Vector2){ GetRandomValue(1, (screenWidth/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.x/2, GetRandomValue(1, (fieldHeight/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.y/2 };
            }
        } else {
            while ((superFruit.position.x == fruit2->position.x) && (superFruit.position.y == fruit2->position.y)) {
                fruit2->position = (Vector2){ GetRandomValue(1, (screenWidth/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.x/2, GetRandomValue(1, (fieldHeight/SQUARE_SIZE) - 2)*SQUARE_SIZE + offset.y/2 };
            }
        }
    }

    // Collision with the fruit (when head is on it already)
    if ((snake[0].position.x < (fruit2->position.x + fruit2->size.x) && (snake[0].position.x + snake[0].size.x) > fruit2->position.x) &&
        (snake[0].position.y < (fruit2->position.y + fruit2->size.y) && (snake[0].position.y + snake[0].size.y) > fruit2->position.y))
    {
        snake[tailLength].position = snakePosition[tailLength - 1];
        tailLength += 1;
        score += 1;
        fruit2->active = false;
        *allowSuperFruit2 = false;
    }
}
