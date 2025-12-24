#include "graph.h"
#include <math.h>
#include <stdio.h>

void Graph_Init(GraphState* state) {
    state->centerX = 0.0;
    state->centerY = 0.0;
    state->scale = 40.0; // 40 pixels per unit default
}

Vector2 Graph_ToScreen(GraphState* state, Vector2 pos, int width, int height) {
    double px = (pos.x - state->centerX) * state->scale + width / 2.0;
    double py = height / 2.0 - (pos.y - state->centerY) * state->scale;
    return (Vector2){ (float)px, (float)py };
}

Vector2 Graph_ToCartesian(GraphState* state, Vector2 pos, int width, int height) {
    double cx = (pos.x - width / 2.0) / state->scale + state->centerX;
    double cy = (height / 2.0 - pos.y) / state->scale + state->centerY;
    return (Vector2){ (float)cx, (float)cy };
}

void Graph_DrawGrid(GraphState* state, int width, int height) {
    // Calculate visible range
    Vector2 topLeft = Graph_ToCartesian(state, (Vector2){0, 0}, width, height);
    Vector2 bottomRight = Graph_ToCartesian(state, (Vector2){(float)width, (float)height}, width, height);

    double minX = topLeft.x;
    double maxX = bottomRight.x;
    double minY = bottomRight.y;
    double maxY = topLeft.y;

    // determine grid step
    // wwe want a line roughly every 100 pixels
    // 100 pixels = 100 / scale units
    double targetStep = 100.0 / state->scale;
    double step = pow(10.0, floor(log10(targetStep)));
    
    // refine step to be 1, 2, or 5 times the power of 10
    if (targetStep / step > 5.0) step *= 5.0;
    else if (targetStep / step > 2.0) step *= 2.0;
    
    Vector2 screenOrigin = Graph_ToScreen(state, (Vector2){0, 0}, width, height);

    
    DrawLine(0, (int)screenOrigin.y, width, (int)screenOrigin.y, BLACK);
    DrawLine((int)screenOrigin.x, 0, (int)screenOrigin.x, height, BLACK);
    
    double startX = floor(minX / step) * step;
    for (double x = startX; x <= maxX; x += step) {
        float screenX = (float)((x - state->centerX) * state->scale + width / 2.0);
        DrawLine((int)screenX, 0, (int)screenX, height, Fade(LIGHTGRAY, 0.5f));
        
        // draw Number
        if (fabs(x) > 1e-10) { 
             char b[32];
             if (fabs(x) >= 1000000 || (fabs(x) < 0.001 && fabs(x) > 1e-10)) {
                 sprintf(b, "%.2e", x);
             } else {
                 sprintf(b, "%.6g", x);
             }
             DrawText(b, (int)screenX + 2, (int)screenOrigin.y + 2, 10, DARKGRAY);
        }
    }

    // draw Horizontal Lines
    double startY = floor(minY / step) * step;
    for (double y = startY; y <= maxY; y += step) {
        float screenY = (float)(height / 2.0 - (y - state->centerY) * state->scale);
        DrawLine(0, (int)screenY, width, (int)screenY, Fade(LIGHTGRAY, 0.5f));
        
         // draw Number
        if (fabs(y) > 1e-10) {
             char b[32];
             if (fabs(y) >= 1000000 || (fabs(y) < 0.001 && fabs(y) > 1e-10)) {
                 sprintf(b, "%.2e", y);
             } else {
                 sprintf(b, "%.6g", y);
             }
             DrawText(b, (int)screenOrigin.x + 2, (int)screenY + 2, 10, DARKGRAY);
        }
    }
}
