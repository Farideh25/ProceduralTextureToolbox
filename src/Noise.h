#pragma once

// Perlin-style 2D gradient noise: smooth, deterministic, in [-1, 1].
// The same (x, y, seed) always gives the same value.
float gradientNoise(float x, float y, int seed);

// Fractal Brownian motion: sum of octaves of noise; each octave has twice the frequency and
// half the amplitude of the previous one. Divided by the total amplitude, so in [-1, 1].
float fbm(float x, float y, int seed, int octaves);

// Like fbm, but sums the absolute noise values, so in [0, 1].
float turbulence(float x, float y, int seed, int octaves);
