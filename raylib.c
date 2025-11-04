/*******************************************************************************************
*
*   raylib - classic game: floppy
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define MAX_TUBES 100
#define FLOPPY_RADIUS 24
#define TUBES_WIDTH 80
#define SCORE_FILE "scores.dat"

//---------------------------- Estructuras ----------------------------
typedef struct Floppy {
    Vector2 position;
    int radius;
    Color color;
} Floppy;

typedef struct Tubes {
    Rectangle rec;
    Color color;
    bool active;
} Tubes;

// Lista enlazada para puntajes
typedef struct Node {
    int score;
    struct Node *next;
} Node;

//---------------------------- Variables globales ----------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;
static int score = 0;
static int hiScore = 0;

static Floppy floppy = {0};
static Tubes tubes[MAX_TUBES * 2] = {0};
static Vector2 tubesPos[MAX_TUBES] = {0};
static float tubesSpeedX = 0;
static bool superfx = false;

// lista enlazada
static Node *scoreList = NULL;

// enum de pantallas
typedef enum GameScreen { MENU, GAMEPLAY, SCORES, EXIT } GameScreen;
static GameScreen currentScreen = MENU;

//---------------------------- Prototipos ----------------------------
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

static void AddScore(int puntos);
static void DrawScores(void);
static void UnloadScores(void);
static void SaveScores(void);
static void LoadScores(void);

//---------------------------- Main ----------------------------
int main(void) {
    InitWindow(screenWidth, screenHeight, "Floppy con Menu, Puntajes y Velocidad");
    LoadScores(); // cargar puntajes previos
    InitGame();
    SetTargetFPS(60);

    while (!WindowShouldClose() && currentScreen != EXIT) {
        UpdateDrawFrame();
    }

    SaveScores();   // guardar antes de salir
    UnloadScores(); // liberar memoria
    UnloadGame();
    CloseWindow();

    return 0;
}

//---------------------------- Funciones de juego ----------------------------
void InitGame(void) {
    floppy.radius = FLOPPY_RADIUS;
    floppy.position = (Vector2){80, screenHeight / 2 - floppy.radius};
    tubesSpeedX = 2.0f;

    for (int i = 0; i < MAX_TUBES; i++) {
        tubesPos[i].x = 400 + 280 * i;
        tubesPos[i].y = -GetRandomValue(0, 120);
    }

    for (int i = 0; i < MAX_TUBES * 2; i += 2) {
        tubes[i].rec.x = tubesPos[i / 2].x;
        tubes[i].rec.y = tubesPos[i / 2].y;
        tubes[i].rec.width = TUBES_WIDTH;
        tubes[i].rec.height = 255;

        tubes[i + 1].rec.x = tubesPos[i / 2].x;
        tubes[i + 1].rec.y = 600 + tubesPos[i / 2].y - 255;
        tubes[i + 1].rec.width = TUBES_WIDTH;
        tubes[i + 1].rec.height = 255;

        tubes[i / 2].active = true;
    }

    score = 0;
    gameOver = false;
    superfx = false;
    pause = false;
}

void UpdateGame(void) {
    if (!gameOver) {
        if (IsKeyPressed('P'))
            pause = !pause;

        if (!pause) {
            for (int i = 0; i < MAX_TUBES; i++)
                tubesPos[i].x -= tubesSpeedX;

            for (int i = 0; i < MAX_TUBES * 2; i += 2) {
                tubes[i].rec.x = tubesPos[i / 2].x;
                tubes[i + 1].rec.x = tubesPos[i / 2].x;
            }

            if (IsKeyDown(KEY_SPACE))
                floppy.position.y -= 3;
            else
                floppy.position.y += 1;

            for (int i = 0; i < MAX_TUBES * 2; i++) {
                if (CheckCollisionCircleRec(floppy.position, floppy.radius, tubes[i].rec)) {
                    gameOver = true;
                    pause = false;
                    AddScore(score); // guarda el puntaje
                } else if ((tubesPos[i / 2].x < floppy.position.x) && tubes[i / 2].active && !gameOver) {
                    score += 100;
                    tubes[i / 2].active = false;
                    superfx = true;

                    // Aumenta la velocidad cada vez que pasa un obstáculo
                    tubesSpeedX += 0.1f;

                    if (score > hiScore)
                        hiScore = score;
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
        } else if (IsKeyPressed(KEY_BACKSPACE)) {
            currentScreen = MENU;
        }
    }
}

void DrawGame(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (!gameOver) {
        DrawCircle(floppy.position.x, floppy.position.y, floppy.radius, DARKGRAY);

        for (int i = 0; i < MAX_TUBES; i++) {
            DrawRectangle(tubes[i * 2].rec.x, tubes[i * 2].rec.y, tubes[i * 2].rec.width, tubes[i * 2].rec.height, GRAY);
            DrawRectangle(tubes[i * 2 + 1].rec.x, tubes[i * 2 + 1].rec.y, tubes[i * 2 + 1].rec.width, tubes[i * 2 + 1].rec.height, GRAY);
        }

        if (superfx) {
            DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            superfx = false;
        }

        DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);
        DrawText(TextFormat("HI-SCORE: %04i", hiScore), 20, 70, 20, LIGHTGRAY);
        DrawText(TextFormat("Velocidad: %.1f", tubesSpeedX), 20, 100, 20, DARKGRAY);

        if (pause)
            DrawText("PAUSA", screenWidth / 2 - MeasureText("PAUSA", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    } else {
        DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 80, 40, GRAY);
        DrawText("ENTER: REINICIAR | BACKSPACE: MENU", screenWidth / 2 - 250, screenHeight / 2, 20, DARKGRAY);
    }

    EndDrawing();
}

void UnloadGame(void) {
    // Nada dinámico
}

//---------------------------- Lista enlazada ----------------------------
void AddScore(int puntos) {
    Node *nuevo = (Node *)malloc(sizeof(Node));
    if (!nuevo)
        return;
    nuevo->score = puntos;
    nuevo->next = scoreList;
    scoreList = nuevo;
}

void DrawScores(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("PUNTAJES GUARDADOS:", screenWidth / 2 - 160, 40, 25, GRAY);

    int y = 100;
    Node *actual = scoreList;
    int i = 1;
    if (!actual)
        DrawText("No hay puntajes guardados.", screenWidth / 2 - 140, 150, 20, DARKGRAY);

    while (actual) {
        DrawText(TextFormat("%d) %d puntos", i, actual->score), screenWidth / 2 - 100, y, 20, GRAY);
        actual = actual->next;
        y += 30;
        i++;
    }

    DrawText("BACKSPACE: Volver al menu", screenWidth / 2 - 140, screenHeight - 40, 20, DARKGRAY);
    EndDrawing();
}

void UnloadScores(void) {
    Node *temp;
    while (scoreList != NULL) {
        temp = scoreList;
        scoreList = scoreList->next;
        free(temp);
    }
}

//---------------------------- Guardar/Cargar Puntajes ----------------------------
void SaveScores(void) {
    FILE *f = fopen(SCORE_FILE, "wb");
    if (!f)
        return;
    Node *actual = scoreList;
    while (actual) {
        fwrite(&actual->score, sizeof(int), 1, f);
        actual = actual->next;
    }
    fclose(f);
}

void LoadScores(void) {
    FILE *f = fopen(SCORE_FILE, "rb");
    if (!f)
        return;
    int puntos;
    while (fread(&puntos, sizeof(int), 1, f) == 1) {
        AddScore(puntos);
    }
    fclose(f);
}

//---------------------------- Motor principal ----------------------------
void UpdateDrawFrame(void) {
    switch (currentScreen) {
    case MENU:
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("MENU PRINCIPAL", screenWidth / 2 - 140, 100, 40, DARKGRAY);
        DrawText("1 - JUGAR", screenWidth / 2 - 60, 180, 30, GRAY);
        DrawText("2 - PUNTAJES", screenWidth / 2 - 60, 220, 30, GRAY);
        DrawText("3 - SALIR", screenWidth / 2 - 60, 260, 30, GRAY);
        EndDrawing();

        if (IsKeyPressed(KEY_ONE))
            currentScreen = GAMEPLAY;
        else if (IsKeyPressed(KEY_TWO))
            currentScreen = SCORES;
        else if (IsKeyPressed(KEY_THREE))
            currentScreen = EXIT;
        break;

    case GAMEPLAY:
        UpdateGame();
        DrawGame();
        break;

    case SCORES:
        DrawScores();
        if (IsKeyPressed(KEY_BACKSPACE))
            currentScreen = MENU;
        break;

    case EXIT:
        break;
    }
}
