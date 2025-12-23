#include "graph.h"
#include <math.h>
#include <stdio.h>

void Graph_Init(GraphState* state) {
    state->minX = -10.0f;
    state->maxX = 10.0f;
    state->minY = -10.0f;
    state->maxY = 10.0f;
    state->offset = (Vector2){ 0, 0 };
    state->zoom = 1.0f;
}

Vector2 Graph_ToScreen(GraphState* state, Vector2 pos, int width, int height) {
    float xRange = state->maxX - state->minX;
    float yRange = state->maxY - state->minY;
    
    float px = (pos.x - state->minX) / xRange * width;
    float py = height - (pos.y - state->minY) / yRange * height;
    
    return (Vector2){ px + state->offset.x, py + state->offset.y };
}

Vector2 Graph_ToCartesian(GraphState* state, Vector2 pos, int width, int height) {
    float xRange = state->maxX - state->minX;
    float yRange = state->maxY - state->minY;
    
    float cx = (pos.x - state->offset.x) / width * xRange + state->minX;
    float cy = (height - (pos.y - state->offset.y)) / height * yRange + state->minY;
    
    return (Vector2){ cx, cy };
}

void Graph_DrawGrid(GraphState* state, int width, int height) {
    float xRange = (state->maxX - state->minX) / state->zoom;
    float yRange = (state->maxY - state->minY) / state->zoom;
    
    // Simple grid step calculation
    float step = powf(10, floorf(log10f(xRange / 5.0f)));
    if (xRange / step > 20) step *= 2;
    if (xRange / step < 5) step /= 2;

    Vector2 zero = Graph_ToScreen(state, (Vector2){0, 0}, width, height);

    // Draw Axes
    DrawLine(0, zero.y, width, zero.y, GRAY);
    DrawLine(zero.x, 0, zero.x, height, GRAY);

    // Draw grid lines
    float startX = floorf(state->minX / step) * step;
    for (float x = startX; x <= state->maxX; x += step) {
        Vector2 p = Graph_ToScreen(state, (Vector2){x, 0}, width, height);
        DrawLine(p.x, 0, p.x, height, Fade(LIGHTGRAY, 0.3f));
        if (fabs(x) > 0.001f) {
            DrawText(TextFormat("%.2f", x), p.x + 2, zero.y + 2, 10, DARKGRAY);
        }
    }

    float startY = floorf(state->minY / step) * step;
    for (float y = startY; y <= state->maxY; y += step) {
        Vector2 p = Graph_ToScreen(state, (Vector2){0, y}, width, height);
        DrawLine(0, p.y, width, p.y, Fade(LIGHTGRAY, 0.3f));
        if (fabs(y) > 0.001f) {
            DrawText(TextFormat("%.2f", y), zero.x + 2, p.y + 2, 10, DARKGRAY);
        }
    }
}
