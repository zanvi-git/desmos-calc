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

void DrawButton(Button *btn, Font font);
bool CheckButton(Button *btn);

void DrawInputField(InputField *input, Font font);
void UpdateInputField(InputField *input);

void InitKeyboard(Keyboard *kb, int screenWidth, int screenHeight);
void DrawKeyboard(Keyboard *kb, Font font);
const char* UpdateKeyboard(Keyboard *kb);


bool DrawSlider(Rectangle rect, const char* text, float* value, float min, float max, Font font);

#endif
