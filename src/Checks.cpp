// Simple correctness checks, run with:  ProceduralTextureToolbox.exe --check
#include "ProceduralTextures.h"
#include <cstdio>
#include <string>

static int failures = 0;

static void check(bool ok, const std::string& description) {
    std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", description.c_str());
    if (!ok)
        failures++;
}

// True if the texture value t stays inside [0, 1] on a grid of sample points.
static bool valuesInRange(const TextureParams& params) {
    for (int y = 0; y <= 100; y++)
        for (int x = 0; x <= 100; x++) {
            float t = textureValue(params, x / 100.0f, y / 100.0f);
            if (t < 0.0f || t > 1.0f)
                return false;
        }
    return true;
}

int runChecks() {
    // Gradient: t goes from 0 (left edge) to 1 (right edge), so the color goes from A to B.
    TextureParams gradient = defaultParams(TEX_GRADIENT);
    check(textureValue(gradient, 0.0f, 0.5f) == 0.0f, "Gradient: t = 0 at the left edge");
    check(textureValue(gradient, 1.0f, 0.5f) == 1.0f, "Gradient: t = 1 at the right edge");

    gradient.colorA = { 255, 0, 0 };  // red
    gradient.colorB = { 0, 0, 255 };  // blue
    uint32_t row[64];
    generateTexture(gradient, row, 64, 1);
    check((row[0] >> 16) > 240 && (row[0] & 0xFF) < 15, "Gradient: first pixel is Color A (red)");
    check((row[63] >> 16) < 15 && (row[63] & 0xFF) > 240, "Gradient: last pixel is Color B (blue)");

    // Every texture must produce values t in [0, 1].
    for (int i = 0; i < TEX_COUNT; i++) {
        TextureType type = (TextureType)i;
        check(valuesInRange(defaultParams(type)), std::string(textureName(type)) + ": values in [0, 1]");
    }

    if (failures == 0)
        std::printf("All checks passed.\n");
    else
        std::printf("%d check(s) FAILED.\n", failures);
    return failures == 0 ? 0 : 1;
}
