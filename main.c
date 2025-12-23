#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include "ui.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MAX_INPUT_CHARS 256
#define MAX_EQUATIONS 5

typedef enum {
    REL_EQ,
    REL_LT,
    REL_GT,
    REL_LE,
    REL_GE
} Relation;

typedef struct {
    InputField input;
    Color color;
    bool visible;
    Relation rel;
    char parsedExpr[256];
} Equation;

void ParseEquation(Equation* eq) {
    const char* text = eq->input.text;
    eq->rel = REL_EQ;
    const char* exprStart = text;

    // Check for "y <", "y >", "y <=", "y >="
    // Simplistic check: assume user types "y" then relation
    // Or maybe just "<", ">" at the start implies "y < ..."
    // Let's support implicit "y=" if no relation found.
    // If user types "< x", it means "y < x".
    
    if (strncmp(text, "y<=", 3) == 0) { eq->rel = REL_LE; exprStart = text + 3; }
    else if (strncmp(text, "y>=", 3) == 0) { eq->rel = REL_GE; exprStart = text + 3; }
    else if (strncmp(text, "y<", 2) == 0) { eq->rel = REL_LT; exprStart = text + 2; }
    else if (strncmp(text, "y>", 2) == 0) { eq->rel = REL_GT; exprStart = text + 2; }
    else if (strncmp(text, "y=", 2) == 0) { eq->rel = REL_EQ; exprStart = text + 2; }
    else if (strncmp(text, "<=", 2) == 0) { eq->rel = REL_LE; exprStart = text + 2; }
    else if (strncmp(text, ">=", 2) == 0) { eq->rel = REL_GE; exprStart = text + 2; }
    else if (text[0] == '<') { eq->rel = REL_LT; exprStart = text + 1; }
    else if (text[0] == '>') { eq->rel = REL_GT; exprStart = text + 1; }
    
    // Copy the rest as the expression
    while (*exprStart == ' ') exprStart++; // Skip spaces
    strcpy(eq->parsedExpr, exprStart);
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "C-Desmos Graphing Calculator");

    GraphState graph;
    Graph_Init(&graph);

    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 32, 0, 250);
    if (font.texture.id == 0) font = GetFontDefault();

    Equation equations[MAX_EQUATIONS];
    Color colors[MAX_EQUATIONS] = { RED, BLUE, GREEN, PURPLE, ORANGE };

    for (int i = 0; i < MAX_EQUATIONS; i++) {
        equations[i].input = (InputField){ 
            .rect = { 10, 10 + i * 50, 300, 40 }, 
            .focused = (i == 0), 
            .letterCount = 0 
        };
        equations[i].color = colors[i];
        equations[i].visible = true;
        equations[i].parsedExpr[0] = '\0';
        equations[i].input.text[0] = '\0';
    }
    
    // Initial equation
    strcpy(equations[0].input.text, "x^2");
    equations[0].input.letterCount = strlen(equations[0].input.text);

    int activeEqIndex = 0;

    Keyboard kb;
    InitKeyboard(&kb, screenWidth, screenHeight);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Handle Mouse Clicks to switch focus
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            for (int i = 0; i < MAX_EQUATIONS; i++) {
                if (CheckCollisionPointRec(mouse, equations[i].input.rect)) {
                    activeEqIndex = i;
                }
            }
        }

        // Update Focus
        for (int i = 0; i < MAX_EQUATIONS; i++) {
            equations[i].input.focused = (i == activeEqIndex);
        }

        // Update Active Input
        InputField* currentInput = &equations[activeEqIndex].input;
        UpdateInputField(currentInput);
        
        const char *kbKey = UpdateKeyboard(&kb);
        if (kbKey != NULL) {
            if (strcmp(kbKey, "C") == 0) {
                currentInput->text[0] = '\0';
                currentInput->letterCount = 0;
            } else if (strcmp(kbKey, "<-") == 0) {
                if (currentInput->letterCount > 0) {
                    currentInput->letterCount--;
                    currentInput->text[currentInput->letterCount] = '\0';
                }
            } else if (strcmp(kbKey, "=") == 0) {
                // Parse on Enter/= key? For now we parse every frame or when changed
            } else if (strcmp(kbKey, " ") != 0) {
                int len = strlen(kbKey);
                if (currentInput->letterCount + len < 255) {
                    strcat(currentInput->text, kbKey);
                    currentInput->letterCount += len;
                }
            }
        }

        if (IsKeyPressed(KEY_K)) kb.visible = !kb.visible;
        
        // Handle Tab to cycle equations
        if (IsKeyPressed(KEY_TAB)) {
            activeEqIndex = (activeEqIndex + 1) % MAX_EQUATIONS;
        }

        // Zoom & Pan
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), (Rectangle){0,0,320, 50*MAX_EQUATIONS}))) {
             // Only pan if not clicking on UI (checking a safe area approx)
             // Actually, simplest is to check if we didn't click an input field
             bool clickedInput = false;
             for(int i=0; i<MAX_EQUATIONS; i++) {
                 if (CheckCollisionPointRec(GetMousePosition(), equations[i].input.rect)) clickedInput = true;
             }
             
             if (!clickedInput || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                graph.offset.x += delta.x;
                graph.offset.y += delta.y;
             }
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

        // Plot Functions
        Parser p;
        for (int eqIdx = 0; eqIdx < MAX_EQUATIONS; eqIdx++) {
            Equation* eq = &equations[eqIdx];
            if (eq->input.letterCount == 0) continue;

            ParseEquation(eq);
            
            // Optimization: If expr is empty, skip
            if (strlen(eq->parsedExpr) == 0) continue;

            Color plotColor = eq->color;
            Color shadeColor = Fade(plotColor, 0.3f);

            Vector2 prevPoint = { 0, 0 };
            bool first = true;
            
            for (int i = 0; i < screenWidth; i++) {
                // Optimization: Step size could be larger for performance, but 1px is fine for now
                Vector2 cart = Graph_ToCartesian(&graph, (Vector2){ (float)i, 0 }, screenWidth, screenHeight);
                
                Parser_Init(&p, eq->parsedExpr);
                float val = (float)Parser_Evaluate(&p, cart.x);
                
                // Convert value back to screen coordinates
                Vector2 screenPoint = Graph_ToScreen(&graph, (Vector2){ cart.x, val }, screenWidth, screenHeight);
                float yScreen = screenPoint.y;

                if (!isnan(val) && !isinf(val)) {
                    // Shading
                    if (eq->rel == REL_LT || eq->rel == REL_LE) {
                        // y < val => Screen Y > yScreen (since screen Y goes down)
                        DrawLine(i, (int)yScreen, i, screenHeight, shadeColor);
                    } else if (eq->rel == REL_GT || eq->rel == REL_GE) {
                        // y > val => Screen Y < yScreen
                        DrawLine(i, 0, i, (int)yScreen, shadeColor);
                    }

                    // Line Drawing
                    if (!first) {
                         // Check for large jumps (asymptotes)
                        if (fabs(screenPoint.y - prevPoint.y) < screenHeight) {
                             DrawLineEx(prevPoint, screenPoint, 2.0f, plotColor);
                        }
                    }
                }
                
                prevPoint = screenPoint;
                first = false;
            }
        }

        // UI
        // Draw sidebar background
        DrawRectangle(0, 0, 320, 10 + MAX_EQUATIONS * 50, Fade(LIGHTGRAY, 0.5f));
        
        for (int i = 0; i < MAX_EQUATIONS; i++) {
             DrawInputField(&equations[i].input, font);
             // Draw color indicator
             DrawRectangle(equations[i].input.rect.x + equations[i].input.rect.width + 5, equations[i].input.rect.y, 10, 40, equations[i].color);
        }

        DrawKeyboard(&kb, font);
        
        DrawTextEx(font, "Press 'K' to toggle keyboard", (Vector2){ 10, (float)screenHeight - 20 }, 10, 2, DARKGRAY);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();

    return 0;
}
