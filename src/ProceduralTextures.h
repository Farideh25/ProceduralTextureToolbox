#pragma once
#include <cstdint>

// Available procedural texture types.
enum TextureType {
    TEX_GRADIENT,
    TEX_SINE,
    TEX_NOISE,
    TEX_COUNT  // number of textures
};

// An RGB color with components in 0..255.
struct Color {
    float r, g, b;
};

// Everything needed to generate one texture.
struct TextureParams {
    TextureType type;
    float frequency;  // sine cycles (Sine) or noise lattice cells (Noise) across the texture
    int seed;         // selects a different, repeatable noise pattern
    Color colorA;  // color where t = 0
    Color colorB;  // color where t = 1
};

// The default parameters ("preset") of a texture.
TextureParams defaultParams(TextureType type);

const char* textureName(TextureType type);
const char* textureFormula(TextureType type);  // short formula shown in the UI

// The value t in [0, 1] of the texture at texture coordinates (u, v), both in [0, 1].
float textureValue(const TextureParams& params, float u, float v);

// Fills a width x height pixel buffer (0x00RRGGBB per pixel) with the texture.
void generateTexture(const TextureParams& params, uint32_t* pixels, int width, int height);
