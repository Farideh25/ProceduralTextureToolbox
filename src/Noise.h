#pragma once

// Perlin-style 2D gradient noise: smooth, deterministic, in [-1, 1].
// The same (x, y, seed) always gives the same value.
float gradientNoise(float x, float y, int seed);
