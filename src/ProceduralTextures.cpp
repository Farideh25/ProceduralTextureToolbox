#include "ProceduralTextures.h"
#include <algorithm>

static const char* NAMES[TEX_COUNT]    = { "Gradient" };
static const char* FORMULAS[TEX_COUNT] = { "t = x" };

const char* textureName(TextureType type) { return NAMES[type]; }
const char* textureFormula(TextureType type) { return FORMULAS[type]; }

TextureParams defaultParams(TextureType type) {
    TextureParams p;
    p.type = type;
    p.colorA = { 20, 40, 110 };   // dark blue
    p.colorB = { 250, 180, 60 };  // orange
    return p;
}

float textureValue(const TextureParams& params, float u, float v) {
    switch (params.type) {
    case TEX_GRADIENT:
        return u;  // 0 at the left edge, 1 at the right edge
    default:
        return 0.0f;
    }
}

// Linear interpolation between two colors: t = 0 gives a, t = 1 gives b.
static Color lerp(Color a, Color b, float t) {
    return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
}

// Packs a color into one pixel: 0x00RRGGBB (the same layout as MiniFB's MFB_RGB).
static uint32_t toPixel(Color c) {
    uint32_t r = (uint32_t)(c.r + 0.5f);
    uint32_t g = (uint32_t)(c.g + 0.5f);
    uint32_t b = (uint32_t)(c.b + 0.5f);
    return (r << 16) | (g << 8) | b;
}

void generateTexture(const TextureParams& params, uint32_t* pixels, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Texture coordinates of the pixel center, both in [0, 1].
            float u = (x + 0.5f) / width;
            float v = (y + 0.5f) / height;

            float t = std::clamp(textureValue(params, u, v), 0.0f, 1.0f);
            pixels[y * width + x] = toPixel(lerp(params.colorA, params.colorB, t));
        }
    }
}
