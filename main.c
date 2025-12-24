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
    char lastInput[256];
    ASTNode* ast;
} Equation;

void ParseEquation(Equation* eq) {
    if (strcmp(eq->input.text, eq->lastInput) == 0 && eq->ast != NULL) return;
    
    strcpy(eq->lastInput, eq->input.text);
    if (eq->ast) { AST_Free(eq->ast); eq->ast = NULL; }

    const char* text = eq->input.text;
    eq->rel = REL_EQ;
    const char* exprStart = text;

    if (strncmp(text, "y<=", 3) == 0) { eq->rel = REL_LE; exprStart = text + 3; }
    else if (strncmp(text, "y>=", 3) == 0) { eq->rel = REL_GE; exprStart = text + 3; }
    else if (strncmp(text, "y<", 2) == 0) { eq->rel = REL_LT; exprStart = text + 2; }
    else if (strncmp(text, "y>", 2) == 0) { eq->rel = REL_GT; exprStart = text + 2; }
    else if (strncmp(text, "y=", 2) == 0) { eq->rel = REL_EQ; exprStart = text + 2; }
    else if (strncmp(text, "<=", 2) == 0) { eq->rel = REL_LE; exprStart = text + 2; }
    else if (strncmp(text, ">=", 2) == 0) { eq->rel = REL_GE; exprStart = text + 2; }
    else if (text[0] == '<') { eq->rel = REL_LT; exprStart = text + 1; }
    else if (text[0] == '>') { eq->rel = REL_GT; exprStart = text + 1; }
    
    while (*exprStart == ' ') exprStart++;
    strcpy(eq->parsedExpr, exprStart);
    
    eq->ast = Parser_Parse(exprStart);
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Graphing Calculator");

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
        equations[i].lastInput[0] = '\0';
        equations[i].ast = NULL;
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
                // Parse on Enter/= key?
            } else if (strcmp(kbKey, " ") != 0) {
                int len = strlen(kbKey);
                if (currentInput->letterCount + len < 255) {
                    strcat(currentInput->text, kbKey);
                    currentInput->letterCount += len;
                }
            }
        }

        if (IsKeyPressed(KEY_K)) kb.visible = !kb.visible;
        
        // handle Tab to cycle equations
        if (IsKeyPressed(KEY_TAB)) {
            activeEqIndex = (activeEqIndex + 1) % MAX_EQUATIONS;
        }

        // Zoom & Pan
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), (Rectangle){0,0,320, 50*MAX_EQUATIONS}))) {
             bool clickedInput = false;
             for(int i=0; i<MAX_EQUATIONS; i++) {
                 if (CheckCollisionPointRec(GetMousePosition(), equations[i].input.rect)) clickedInput = true;
             }
             
             if (!clickedInput || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                graph.centerX -= delta.x / graph.scale;
                graph.centerY += delta.y / graph.scale; 
             }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mousePos = GetMousePosition();
            Vector2 mouseWorldBefore = Graph_ToCartesian(&graph, mousePos, screenWidth, screenHeight);
            
            float scaleFactor = 1.1f;
            if (wheel > 0) graph.scale *= scaleFactor;
            else graph.scale /= scaleFactor;
            
            // adjust center so that mouseWorld remains under mousePos
            Vector2 mouseWorldAfter = Graph_ToCartesian(&graph, mousePos, screenWidth, screenHeight);
            graph.centerX += (mouseWorldBefore.x - mouseWorldAfter.x);
            graph.centerY += (mouseWorldBefore.y - mouseWorldAfter.y);
        }

        // draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Graph_DrawGrid(&graph, screenWidth, screenHeight);
        
        // Plot Functions
        EvalContext ctx;
        ctx.y = 0; 
        ctx.t = 0; // Default time to 0


        for (int eqIdx = 0; eqIdx < MAX_EQUATIONS; eqIdx++) {
            Equation* eq = &equations[eqIdx];
            if (eq->input.letterCount == 0) continue;

            ParseEquation(eq);
            
            // Optimization: If expr is empty or ast is null, skip
            if (!eq->ast) continue;

            Color plotColor = eq->color;
            Color shadeColor = Fade(plotColor, 0.3f);

            Vector2 prevPoint = { 0, 0 };
            bool first = true;
            
            for (int i = 0; i < screenWidth; i++) {
                // High precision conversion for infinite zoom
                double worldX = ((double)i - screenWidth / 2.0) / graph.scale + graph.centerX;
                
                ctx.x = worldX;
                double val = AST_Evaluate(eq->ast, &ctx);
                
                // Convert value back to screen coordinates manually for precision intermediate
                // screenY = height/2 - (val - centerY) * scale
                double screenY = (double)screenHeight / 2.0 - (val - graph.centerY) * graph.scale;
                
                Vector2 screenPoint = { (float)i, (float)screenY };
                
                if (!isnan(val) && !isinf(val)) {
                    // Shading
                    if (eq->rel == REL_LT || eq->rel == REL_LE) {
                        // y < val => Screen Y > screenY
                        DrawLine(i, (int)screenY, i, screenHeight, shadeColor);
                    } else if (eq->rel == REL_GT || eq->rel == REL_GE) {
                        // y > val => Screen Y < screenY
                        DrawLine(i, 0, i, (int)screenY, shadeColor);
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
        DrawRectangle(0, 0, 320, screenHeight, Fade(LIGHTGRAY, 0.5f)); // Extend sidebar background to full height
        
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
