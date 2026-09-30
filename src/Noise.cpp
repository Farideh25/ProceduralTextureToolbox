#include "Noise.h"
#include <cmath>
#include <cstdint>

// Pseudo-random 32-bit value for lattice point (x, y).
// The inputs are combined with large constants and then the bits are mixed.
// Unsigned arithmetic, so overflow wraps around and the result is well defined.
static uint32_t hash(int x, int y, int seed) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ (uint32_t)seed * 83492791u;
    h ^= h >> 16;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;
    return h;
}

// 8 unit gradient directions, 45 degrees apart.
static const float GRADIENTS[8][2] = {
    { 1, 0 },  { 0.7071f, 0.7071f },   { 0, 1 },  { -0.7071f, 0.7071f },
    { -1, 0 }, { -0.7071f, -0.7071f }, { 0, -1 }, { 0.7071f, -0.7071f },
};

// Dot product of the gradient at lattice point (ix, iy) with the offset (dx, dy) from that point.
static float corner(int ix, int iy, int seed, float dx, float dy) {
    const float* g = GRADIENTS[hash(ix, iy, seed) & 7];
    return g[0] * dx + g[1] * dy;
}

// Interpolation weight 6t^5 - 15t^4 + 10t^3: zero slope at t = 0 and t = 1.
static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float gradientNoise(float x, float y, int seed) {
    // Lattice cell containing (x, y), and the position inside the cell (in [0, 1)).
    int ix = (int)std::floor(x);
    int iy = (int)std::floor(y);
    float fx = x - (float)ix;
    float fy = y - (float)iy;

    // Contributions of the gradients at the four cell corners.
    float n00 = corner(ix,     iy,     seed, fx,        fy);
    float n10 = corner(ix + 1, iy,     seed, fx - 1.0f, fy);
    float n01 = corner(ix,     iy + 1, seed, fx,        fy - 1.0f);
    float n11 = corner(ix + 1, iy + 1, seed, fx - 1.0f, fy - 1.0f);

    // Smooth interpolation between the four contributions.
    float u = fade(fx);
    float v = fade(fy);
    float n = lerp(lerp(n00, n10, u), lerp(n01, n11, u), v);

    // Offset components lie in [-1, 1], so each offset is at most sqrt(2) long and, with unit
    // gradients, each contribution is at most sqrt(2) in magnitude. n is a weighted average of
    // the contributions, so n / sqrt(2) lies in [-1, 1].
    return n / 1.41421356f;
}

// x and y are already multiplied by the texture frequency; octave i samples at 2^i times that.
float fbm(float x, float y, int seed, int octaves) {
    float sum = 0.0f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;
    float frequency = 1.0f;  // relative to the texture frequency
    for (int i = 0; i < octaves; i++) {
        sum += amplitude * gradientNoise(frequency * x, frequency * y, seed);
        totalAmplitude += amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }
    return sum / totalAmplitude;
}

// Same octaves as fbm, with the absolute value of each noise layer.
float turbulence(float x, float y, int seed, int octaves) {
    float sum = 0.0f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;
    float frequency = 1.0f;  // relative to the texture frequency
    for (int i = 0; i < octaves; i++) {
        sum += amplitude * std::fabs(gradientNoise(frequency * x, frequency * y, seed));
        totalAmplitude += amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }
    return sum / totalAmplitude;
}
