#pragma once 

#include "raylib.h"
#include "raymath.h"
#include <vector>

Vector2 RotacaoPonto(Vector2 posicao, float angulo_);
Vector2 RotatePoint(Vector2 pivot, Vector2 point, float angle);
Vector2 FindIntersection(Vector2 rectSize, Vector2 size);

void drawSeta(Vector2 psi, Vector2 psf, float esp, float compt, float zoom, Color corSeta);
void drawForca(float x, float y, float fx, float fy, float zoom, Color cor);
void drawMoment(Vector2 position, float radius, bool isAnticlockwise, float arrowLength, float arrowWidth, float lineWidth, Color color, Camera2D camera, const char *annotation);
void drawFixedSizeAnnotadedMoment(Vector2 position, float moment, float radius, float arrowLength, float arrowWidth, float lineWidth, Color color, Camera2D cam);