#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"

typedef struct Renderer
{
    RenderTexture2D MainRenderTexture;
    Vector2 MainRenderTextureOrigin;
    Rectangle MainRenderTextureSourceRec;
    Rectangle MainRenderTextureDestRec;
    double virtualRatio;
} Renderer;

#endif // RENDERER_H
