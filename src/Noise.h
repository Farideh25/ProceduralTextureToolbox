#pragma once

// Perlin-style 2D gradient noise: smooth, deterministic, in [-1, 1].
// The same (x, y, seed) always gives the same value.
float gradientNoise(float x, float y, int seed);

// Fractal Brownian motion: sum of noise octaves.
// Lacunarity controls frequency growth and gain controls amplitude decay.
// Divided by the total amplitude, so the result remains in [-1, 1].
float fbm(float x, float y, int seed, int octaves,
          float lacunarity = 2.0f, float gain = 0.5f);

// Like fbm, but sums the absolute noise values, so in [0, 1].
float turbulence(float x, float y, int seed, int octaves,
                 float lacunarity = 2.0f, float gain = 0.5f);