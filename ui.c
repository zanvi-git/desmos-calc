#include "ui.h"
#include <string.h>

void DrawButton(Button *btn) {
    Color drawColor = btn->color;
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect)) {
        drawColor = btn->hoverColor;
    }
    
    DrawRectangleRec(btn->rect, drawColor);
    DrawRectangleLinesEx(btn->rect, 1, DARKGRAY);
    
    int fontSize = 20;
    int textWidth = MeasureText(btn->text, fontSize);
    DrawText(btn->text, 
             btn->rect.x + (btn->rect.width - textWidth) / 2, 
             btn->rect.y + (btn->rect.height - fontSize) / 2, 
             fontSize, btn->textColor);
}

bool CheckButton(Button *btn) {
    if (CheckCollisionPointRec(GetMousePosition(), btn->rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }
    return false;
}

void DrawInputField(InputField *input) {
    DrawRectangleRec(input->rect, WHITE);
    DrawRectangleLinesEx(input->rect, 2, input->focused ? BLUE : DARKGRAY);
    
    DrawText(input->text, input->rect.x + 5, input->rect.y + 10, 20, BLACK);
    
    if (input->focused) {
        // Simple blinking cursor
        if (((int)(GetTime() * 2) % 2) == 0) {
            int textWidth = MeasureText(input->text, 20);
            DrawRectangle(input->rect.x + 5 + textWidth, input->rect.y + 10, 2, 20, BLACK);
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

void DrawKeyboard(Keyboard *kb) {
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
            
            int fontSize = 16;
            int textWidth = MeasureText(kb->keys[i][j], fontSize);
            DrawText(kb->keys[i][j], btnRect.x + (btnRect.width - textWidth)/2, btnRect.y + (btnRect.height - fontSize)/2, fontSize, BLACK);
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
