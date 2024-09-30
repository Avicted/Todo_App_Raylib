// Header guard
#ifndef INCLUDES_H
#define INCLUDES_H

// Standard library includes
#include <stdio.h>

// Custom includes
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Own includes
#include "renderer.h"

// Macros
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif // INCLUDES_H