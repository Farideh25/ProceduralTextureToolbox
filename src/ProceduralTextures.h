#pragma once
#include <cstdint>

// Available procedural texture types.
enum TextureType {
    TEX_GRADIENT,
    TEX_SINE,
    TEX_NOISE,
    TEX_FBM,
    TEX_TURBULENCE,
    TEX_MARBLE,
    TEX_WOOD,
    TEX_CLOUDS,
    TEX_COUNT  // number of textures
};

// An RGB color with components in 0..255.
struct Color {
    float r, g, b;
};

// Everything needed to generate one texture.
struct TextureParams {
    TextureType type;
    float frequency;  // pattern frequency or base noise frequency, depending on the texture
    int seed;         // selects a different, repeatable noise pattern
    int octaves;      // number of noise layers (fBm, Turbulence, Marble, Wood, Clouds)
    float lacunarity; // frequency multiplier between noise octaves
    float gain;       // amplitude multiplier between noise octaves
    float distortion;  // strength of the turbulence distortion (Marble, Wood)
    Color colorA;  // color where t = 0
    Color colorB;  // color where t = 1
};

// The default parameters ("preset") of a texture.
TextureParams defaultParams(TextureType type);

const char* textureName(TextureType type);
const char* textureFormula(TextureType type);  // short formula or description shown in the UI

// The value t in [0, 1] of the texture at texture coordinates (u, v), both in [0, 1].
float textureValue(const TextureParams& params, float u, float v);

// Fills a width x height pixel buffer (0x00RRGGBB per pixel) with the texture.
void generateTexture(const TextureParams& params, uint32_t* pixels, int width, int height);
