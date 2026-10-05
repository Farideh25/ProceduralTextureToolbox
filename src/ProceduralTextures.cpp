#include "ProceduralTextures.h"
#include "Noise.h"
#include <algorithm>
#include <cmath>

static const float PI = 3.14159265f;

static const char* NAMES[TEX_COUNT]    = { "Gradient", "Sine", "Noise", "fBm", "Turbulence", "Marble", "Wood", "Clouds" };
static const char* FORMULAS[TEX_COUNT] = {
    "t = x",
    "t = 0.5 + 0.5 sin(2 pi f x)",
    "t = 0.5 + 0.5 noise(f x, f y, seed)",
    "weighted sum of noise octaves",
    "weighted sum of |noise| octaves",
    "sine with turbulence distortion",
    "distorted radial rings",
    "fBm mapped to sky colors",
};

const char* textureName(TextureType type) { return NAMES[type]; }
const char* textureFormula(TextureType type) { return FORMULAS[type]; }

TextureParams defaultParams(TextureType type) {
    TextureParams p;
    p.type = type;
    p.frequency = 8.0f;
    p.seed = 1;
    p.octaves = 4;
    p.distortion = 2.0f;
    p.colorA = { 20, 40, 110 };   // dark blue
    p.colorB = { 250, 180, 60 };  // orange
    if (type == TEX_NOISE || type == TEX_FBM || type == TEX_TURBULENCE) {
        p.colorA = { 0, 0, 0 };        // black
        p.colorB = { 255, 255, 255 };  // white
    } else if (type == TEX_MARBLE) {
        p.colorA = { 60, 60, 70 };     // dark stone
        p.colorB = { 235, 232, 225 };  // light stone
    } else if (type == TEX_WOOD) {
        p.colorA = { 95, 55, 25 };     // dark brown
        p.colorB = { 205, 155, 95 };   // light brown
    } else if (type == TEX_CLOUDS) {
        p.colorA = { 90, 150, 220 };   // sky blue
        p.colorB = { 255, 255, 255 };  // white
    }
    return p;
}

float textureValue(const TextureParams& params, float u, float v) {
    switch (params.type) {
    case TEX_GRADIENT:
        return u;  // 0 at the left edge, 1 at the right edge
    case TEX_SINE:
        return 0.5f + 0.5f * std::sin(2.0f * PI * params.frequency * u);
    case TEX_NOISE:
        return 0.5f + 0.5f * gradientNoise(
            params.frequency * u,
            params.frequency * v,
            params.seed);
    case TEX_FBM:
        return 0.5f + 0.5f * fbm(
            params.frequency * u,
            params.frequency * v,
            params.seed,
            params.octaves);
    case TEX_TURBULENCE:
        return turbulence(
            params.frequency * u,
            params.frequency * v,
            params.seed,
            params.octaves);
    case TEX_MARBLE: {
        // Sine stripes whose phase is shifted by turbulence.
        float noise = turbulence(
            params.frequency * u,
            params.frequency * v,
            params.seed,
            params.octaves);
        float phase = params.frequency * u + params.distortion * noise;
        return 0.5f + 0.5f * std::sin(2.0f * PI * phase);
    }
    case TEX_WOOD: {
        // Sine rings around the texture center; turbulence shifts the radius.
        float dx = u - 0.5f;
        float dy = v - 0.5f;
        float radius = std::sqrt(dx * dx + dy * dy);
        float noise = turbulence(
            params.frequency * u,
            params.frequency * v,
            params.seed,
            params.octaves);
        float warpedRadius = radius + params.distortion * 0.05f * noise;  // at most 5% of the texture per unit of distortion
        return 0.5f + 0.5f * std::sin(2.0f * PI * params.frequency * warpedRadius);
    }
    case TEX_CLOUDS: {
    float t = 0.5f + 0.5f * fbm(
        params.frequency * u,
        params.frequency * v,
        params.seed,
        params.octaves);

    // Expand the contrast around the midpoint so the cloud structure is easier to see.
    return std::clamp(0.5f + 1.35f * (t - 0.5f), 0.0f, 1.0f);
}
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
