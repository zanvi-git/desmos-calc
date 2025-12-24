#ifndef GRAPH_H
#define GRAPH_H

#include "raylib.h"

typedef struct {
    double centerX;
    double centerY;
    double scale; 
} GraphState;

void Graph_Init(GraphState* state);
Vector2 Graph_ToScreen(GraphState* state, Vector2 pos, int width, int height);
Vector2 Graph_ToCartesian(GraphState* state, Vector2 pos, int width, int height);
void Graph_DrawGrid(GraphState* state, int width, int height);

#endif
