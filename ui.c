#include "ui.h"
#include <string.h>
#include <stdio.h>

void DrawButton(Button *btn, Font font) {
    Color drawColor = btn->color;
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect)) {
        drawColor = btn->hoverColor;
    }
    
    DrawRectangleRec(btn->rect, drawColor);
    DrawRectangleLinesEx(btn->rect, 1, DARKGRAY);
    
    float fontSize = 20;
    Vector2 textSize = MeasureTextEx(font, btn->text, fontSize, 2);
    DrawTextEx(font, btn->text, 
               (Vector2){ btn->rect.x + (btn->rect.width - textSize.x) / 2, 
                          btn->rect.y + (btn->rect.height - textSize.y) / 2 }, 
               fontSize, 2, btn->textColor);
}

bool CheckButton(Button *btn) {
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }
    return false;
}

void DrawInputField(InputField *input, Font font) {
    DrawRectangleRec(input->rect, WHITE);
    DrawRectangleLinesEx(input->rect, 2, input->focused ? BLUE : DARKGRAY);
    
    float fontSize = 20;
    DrawTextEx(font, input->text, (Vector2){ input->rect.x + 5, input->rect.y + 10 }, fontSize, 2, BLACK);
    
    if (input->focused) {
        // simple blinking cursor kinda aesthetic
        if (((int)(GetTime() * 2) % 2) == 0) {
            Vector2 textSize = MeasureTextEx(font, input->text, fontSize, 2);
            DrawRectangle(input->rect.x + 5 + textSize.x, input->rect.y + 10, 2, fontSize, BLACK);
        }
    }
}

void UpdateInputField(InputField *input) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(GetMousePosition(), input->rect)) {
            input->focused = true;
        } else {
            input->focused = false;
        }
    }

    if (input->focused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (input->letterCount < 255)) {
                input->text[input->letterCount] = (char)key;
                input->text[input->letterCount + 1] = '\0';
                input->letterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (input->letterCount > 0) {
                input->letterCount--;
                input->text[input->letterCount] = '\0';
            }
        }
    }
}

void InitKeyboard(Keyboard *kb, int screenWidth, int screenHeight) {
    kb->visible = true;
    float kbHeight = 200;
    kb->rect = (Rectangle){ 0, screenHeight - kbHeight, screenWidth, kbHeight };
    
    const char *layout[4][10] = {
        {"7", "8", "9", "/", "sin", "cos", "tan", "(", ")", "C"},
        {"4", "5", "6", "*", "arcsin", "arccos", "arctan", "x", "^", "<-"},
        {"1", "2", "3", "-", "log", "ln", "sqrt", "y", "e", "="},
        {"0", ".", "pi", "+", "abs", "mod", "exp", " ", " ", " "}
    };

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 10; j++) {
            kb->keys[i][j] = layout[i][j];
        }
    }
}

void DrawKeyboard(Keyboard *kb, Font font) {
    if (!kb->visible) return;

    DrawRectangleRec(kb->rect, (Color){ 240, 240, 240, 255 });
    DrawLine(kb->rect.x, kb->rect.y, kb->rect.x + kb->rect.width, kb->rect.y, DARKGRAY);

    float btnWidth = kb->rect.width / 10.0f;
    float btnHeight = kb->rect.height / 4.0f;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            if (kb->keys[i][j][0] == ' ' && kb->keys[i][j][1] == '\0') continue;

            Rectangle btnRect = { kb->rect.x + j * btnWidth + 2, kb->rect.y + i * btnHeight + 2, btnWidth - 4, btnHeight - 4 };
            bool hover = CheckCollisionPointRec(GetMousePosition(), btnRect);
            
            DrawRectangleRounded(btnRect, 0.2f, 8, hover ? LIGHTGRAY : WHITE);
            DrawRectangleRoundedLines(btnRect, 0.2f, 8, DARKGRAY);
            
            float fontSize = 16;
            Vector2 textSize = MeasureTextEx(font, kb->keys[i][j], fontSize, 2);
            DrawTextEx(font, kb->keys[i][j], 
                       (Vector2){ btnRect.x + (btnRect.width - textSize.x)/2, 
                                  btnRect.y + (btnRect.height - textSize.y)/2 }, 
                       fontSize, 2, BLACK);
        }
    }
}

const char* UpdateKeyboard(Keyboard *kb) {
    if (!kb->visible) return NULL;

    float btnWidth = kb->rect.width / 10.0f;
    float btnHeight = kb->rect.height / 4.0f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 10; j++) {
                Rectangle btnRect = { kb->rect.x + j * btnWidth + 2, kb->rect.y + i * btnHeight + 2, btnWidth - 4, btnHeight - 4 };
                if (CheckCollisionPointRec(GetMousePosition(), btnRect)) {
                    return kb->keys[i][j];
                }
            }
        }
    }
    return NULL;
}

bool DrawSlider(Rectangle rect, const char* text, float* value, float min, float max, Font font) {
    bool changed = false;
    
    // draw Label
    if (text) {
        DrawTextEx(font, text, (Vector2){ rect.x, rect.y - 20 }, 16, 1, DARKGRAY);
    }
    
    // draw background
    DrawRectangleRounded(rect, 0.5f, 4, LIGHTGRAY);
    
    // handle input
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, (Rectangle){ rect.x - 10, rect.y - 10, rect.width + 20, rect.height + 20 })) {
            float mousePos = mouse.x - rect.x;
            float normalized = mousePos / rect.width;
            if (normalized < 0) normalized = 0;
            if (normalized > 1) normalized = 1;
            
            *value = min + normalized * (max - min);
            changed = true;
        }
    }
    
    // draw Knob
    float normalized = (*value - min) / (max - min);
    float knobX = rect.x + normalized * rect.width;
    DrawCircle((int)knobX, (int)(rect.y + rect.height/2), rect.height, DARKGRAY);
    DrawCircle((int)knobX, (int)(rect.y + rect.height/2), rect.height - 2, WHITE);
    
    // draw Value
    char buf[32];
    sprintf(buf, "%.2f", *value);
    // DrawTextEx(font, buf, (Vector2){ rect.x + rect.width + 10, rect.y }, 16, 1, BLACK);
    
    // Let caller draw value if they want, or just verify logic.
    return changed;
}
