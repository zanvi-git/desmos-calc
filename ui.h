#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Rectangle rect;
    const char *text;
    Color color;
    Color hoverColor;
    Color textColor;
    bool clicked;
} Button;

typedef struct {
    Rectangle rect;
    char text[256];
    int letterCount;
    bool focused;
    int cursorPosition;
} InputField;

typedef struct {
    Rectangle rect;
    bool visible;
    const char *keys[4][10];
} Keyboard;

void DrawButton(Button *btn);
bool CheckButton(Button *btn);

void DrawInputField(InputField *input);
void UpdateInputField(InputField *input);

void InitKeyboard(Keyboard *kb, int screenWidth, int screenHeight);
void DrawKeyboard(Keyboard *kb);
const char* UpdateKeyboard(Keyboard *kb);

#endif
