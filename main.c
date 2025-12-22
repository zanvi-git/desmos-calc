#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include <string.h>
#include <math.h>

#define MAX_INPUT_CHARS 256

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "C-Desmos Graphing Calculator");

    GraphState graph;
    Graph_Init(&graph);

    char input[MAX_INPUT_CHARS] = "x^2";
    int letterCount = strlen(input);
    
    Rectangle textBox = { 10, 10, 225, 30 };
    bool mouseOnText = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
        else mouseOnText = false;

        if (mouseOnText) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS - 1)) {
                    input[letterCount] = (char)key;
                    input[letterCount + 1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                input[letterCount] = '\0';
            }
        } else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        // Zoom & Pan
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            graph.offset.x += delta.x;
            graph.offset.y += delta.y;
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            graph.zoom += wheel * 0.1f;
            if (graph.zoom < 0.1f) graph.zoom = 0.1f;
        }

        // Draw :]
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Graph_DrawGrid(&graph, screenWidth, screenHeight);

       
        Parser p;
        if (letterCount > 0) {
            Vector2 prevPoint = { 0, 0 };
            bool first = true;
            
            for (int i = 0; i < screenWidth; i++) {
                Vector2 cart = Graph_ToCartesian(&graph, (Vector2){ (float)i, 0 }, screenWidth, screenHeight);
                Parser_Init(&p, input);
                float val = (float)Parser_Evaluate(&p, cart.x);
                Vector2 screenPoint = Graph_ToScreen(&graph, (Vector2){ cart.x, val }, screenWidth, screenHeight);

                if (!first) {
                    if (!isnan(val) && !isinf(val)) {
                         DrawLineEx(prevPoint, screenPoint, 2.0f, BLUE);
                    }
                }
                prevPoint = screenPoint;
                first = false;
            }
        }

        // UI
        DrawRectangleRec(textBox, LIGHTGRAY);
        if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);

        DrawText(input, (int)textBox.x + 5, (int)textBox.y + 8, 20, MAROON);
        DrawText(TextFormat("INPUT: %i/%i", letterCount, MAX_INPUT_CHARS), 10, 45, 10, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
