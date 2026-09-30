// Simple correctness checks, run with:  ProceduralTextureToolbox.exe --check
#include "ProceduralTextures.h"
#include "Noise.h"
#include <cmath>
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

    // Sine: t = 0.5 + 0.5 sin(2 pi f u).
    TextureParams sine = defaultParams(TEX_SINE);
    sine.frequency = 1.0f;
    check(std::fabs(textureValue(sine, 0.0f, 0.5f) - 0.5f) < 1e-5f &&
          std::fabs(textureValue(sine, 0.25f, 0.5f) - 1.0f) < 1e-5f &&
          std::fabs(textureValue(sine, 0.75f, 0.5f) - 0.0f) < 1e-5f,
          "Sine (f = 1): t(0) = 0.5, t(0.25) = 1, t(0.75) = 0");

    sine.frequency = 4.0f;
    const float points[] = { 0.05f, 0.2f, 0.35f, 0.6f };
    bool repeats = true;
    for (float u : points)
        if (std::fabs(textureValue(sine, u, 0.5f) - textureValue(sine, u + 0.25f, 0.5f)) > 1e-4f)
            repeats = false;
    check(repeats, "Sine (f = 4): t(u) = t(u + 1/4)");

    // Noise at fixed sample points that are not on the integer lattice.
    const float samples[][2] = { { 0.3f, 0.7f }, { 1.6f, 2.2f }, { 3.4f, 0.9f }, { 5.1f, 4.8f }, { 7.7f, 6.3f } };
    bool sameSeedSameValue = true;
    bool otherSeedChangesValue = false;
    for (const auto& s : samples) {
        if (gradientNoise(s[0], s[1], 1) != gradientNoise(s[0], s[1], 1))
            sameSeedSameValue = false;
        if (gradientNoise(s[0], s[1], 1) != gradientNoise(s[0], s[1], 2))
            otherSeedChangesValue = true;
    }
    check(sameSeedSameValue, "Noise: the same seed gives the same values");
    check(otherSeedChangesValue, "Noise: seed 2 changes at least one value compared to seed 1");

    // Noise is exactly 0 at lattice points: the offset to that corner is (0, 0) and fade(0) = 0.
    bool zeroAtLattice = true;
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (gradientNoise((float)x, (float)y, 1) != 0.0f)
                zeroAtLattice = false;
    check(zeroAtLattice, "Noise: exactly 0 at lattice points");

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
