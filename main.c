#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include "ui.h"
#include <string.h>
#include <math.h>

#define MAX_INPUT_CHARS 256

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "C-Desmos Graphing Calculator");

    GraphState graph;
    Graph_Init(&graph);

    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 32, 0, 250);
    if (font.texture.id == 0) font = GetFontDefault();

    InputField equationInput = { .rect = { 10, 10, 300, 40 }, .focused = false, .letterCount = 0 };
    strcpy(equationInput.text, "x^2");
    equationInput.letterCount = strlen(equationInput.text);

    Keyboard kb;
    InitKeyboard(&kb, screenWidth, screenHeight);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update UI
        UpdateInputField(&equationInput);
        const char *kbKey = UpdateKeyboard(&kb);
        if (kbKey != NULL) {
            if (strcmp(kbKey, "C") == 0) {
                equationInput.text[0] = '\0';
                equationInput.letterCount = 0;
            } else if (strcmp(kbKey, "<-") == 0) {
                if (equationInput.letterCount > 0) {
                    equationInput.letterCount--;
                    equationInput.text[equationInput.letterCount] = '\0';
                }
            } else if (strcmp(kbKey, "=") == 0) {
                // Could trigger evaluation or something
            } else if (strcmp(kbKey, " ") != 0) {
                int len = strlen(kbKey);
                if (equationInput.letterCount + len < 255) {
                    strcat(equationInput.text, kbKey);
                    equationInput.letterCount += len;
                }
            }
        }

        if (IsKeyPressed(KEY_K)) kb.visible = !kb.visible;

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

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Graph_DrawGrid(&graph, screenWidth, screenHeight);

        // Plot Function
        Parser p;
        if (equationInput.letterCount > 0) {
            Vector2 prevPoint = { 0, 0 };
            bool first = true;
            
            for (int i = 0; i < screenWidth; i++) {
                Vector2 cart = Graph_ToCartesian(&graph, (Vector2){ (float)i, 0 }, screenWidth, screenHeight);
                Parser_Init(&p, equationInput.text);
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
        DrawInputField(&equationInput, font);
        DrawKeyboard(&kb, font);
        
        DrawTextEx(font, "Press 'K' to toggle keyboard", (Vector2){ 10, (float)screenHeight - 20 }, 10, 2, DARKGRAY);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();

    return 0;
}
