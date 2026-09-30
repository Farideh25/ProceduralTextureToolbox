#pragma once
#include <cstdint>

extern "C" {
#include "microui.h"
}

struct mfb_window;

// Creates the MicroUI context and forwards the window's mouse events to it.
mu_Context* uiInit(mfb_window* window);

// Draws MicroUI's command list (rectangles, text, icons) into our pixel buffer.
void uiRender(mu_Context* ctx, uint32_t* pixels, int width, int height);
