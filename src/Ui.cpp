#include "Ui.h"
#include <MiniFB.h>
#include <algorithm>
#include <cstring>

// MicroUI's font and icons, compiled as C in ui_atlas.c.
extern "C" {
unsigned char ui_atlas_alpha(int x, int y);
mu_Rect ui_atlas_char(unsigned char c);
mu_Rect ui_atlas_icon(int id);
}

// Store the MicroUI context statically instead of on the stack.
static mu_Context g_ctx;

// ---- Input: MiniFB mouse events -> MicroUI ----

static void onMouseMove(mfb_window*, int x, int y) {
    mu_input_mousemove(&g_ctx, x, y);
}

static void onMouseButton(mfb_window* window, mfb_mouse_button button, mfb_key_mod, bool isPressed) {
    if (button != MFB_MOUSE_LEFT)
        return;
    int x = mfb_get_mouse_x(window);
    int y = mfb_get_mouse_y(window);
    if (isPressed)
        mu_input_mousedown(&g_ctx, x, y, MU_MOUSE_LEFT);
    else
        mu_input_mouseup(&g_ctx, x, y, MU_MOUSE_LEFT);
}

// MicroUI asks for the size of text to lay out its widgets.
static int textWidth(mu_Font, const char* text, int len) {
    if (len < 0)
        len = (int)strlen(text);
    int width = 0;
    for (int i = 0; i < len && text[i]; i++)
        width += ui_atlas_char((unsigned char)text[i]).w;
    return width;
}

static int textHeight(mu_Font) {
    return 18;
}

mu_Context* uiInit(mfb_window* window) {
    mu_init(&g_ctx);
    g_ctx.text_width = textWidth;
    g_ctx.text_height = textHeight;
    mfb_set_mouse_move_callback(window, onMouseMove);
    mfb_set_mouse_button_callback(window, onMouseButton);
    return &g_ctx;
}

// ---- Rendering: MicroUI draw commands -> our pixel buffer ----

static uint32_t* g_pixels;
static int g_width;
static mu_Rect g_clip;  // only pixels inside this rectangle may be changed

// Blends color c into pixel (x, y). alpha (0..255) is the coverage, e.g. from a font glyph.
static void blendPixel(int x, int y, mu_Color c, int alpha) {
    if (x < g_clip.x || y < g_clip.y || x >= g_clip.x + g_clip.w || y >= g_clip.y + g_clip.h)
        return;
    int a = alpha * c.a / 255;
    uint32_t& dst = g_pixels[y * g_width + x];
    int r = (dst >> 16) & 0xFF, g = (dst >> 8) & 0xFF, b = dst & 0xFF;
    r += (c.r - r) * a / 255;
    g += (c.g - g) * a / 255;
    b += (c.b - b) * a / 255;
    dst = (r << 16) | (g << 8) | b;
}

static void drawRect(mu_Rect rect, mu_Color c) {
    for (int y = rect.y; y < rect.y + rect.h; y++)
        for (int x = rect.x; x < rect.x + rect.w; x++)
            blendPixel(x, y, c, 255);
}

// Draws a part of the atlas image (a character or an icon) at (x, y), tinted with color c.
static void drawAtlasImage(mu_Rect src, int x, int y, mu_Color c) {
    for (int j = 0; j < src.h; j++)
        for (int i = 0; i < src.w; i++)
            blendPixel(x + i, y + j, c, ui_atlas_alpha(src.x + i, src.y + j));
}

static void drawText(const char* text, mu_Vec2 pos, mu_Color c) {
    for (const char* p = text; *p; p++) {
        mu_Rect glyph = ui_atlas_char((unsigned char)*p);
        drawAtlasImage(glyph, pos.x, pos.y, c);
        pos.x += glyph.w;
    }
}

static void drawIcon(int id, mu_Rect rect, mu_Color c) {
    mu_Rect icon = ui_atlas_icon(id);
    drawAtlasImage(icon, rect.x + (rect.w - icon.w) / 2, rect.y + (rect.h - icon.h) / 2, c);
}

static mu_Rect intersect(mu_Rect a, mu_Rect b) {
    int x1 = std::max(a.x, b.x), y1 = std::max(a.y, b.y);
    int x2 = std::min(a.x + a.w, b.x + b.w), y2 = std::min(a.y + a.h, b.y + b.h);
    return mu_rect(x1, y1, std::max(0, x2 - x1), std::max(0, y2 - y1));
}

void uiRender(mu_Context* ctx, uint32_t* pixels, int width, int height) {
    g_pixels = pixels;
    g_width = width;
    mu_Rect screen = mu_rect(0, 0, width, height);
    g_clip = screen;

    mu_Command* cmd = nullptr;
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
        case MU_COMMAND_RECT: drawRect(cmd->rect.rect, cmd->rect.color); break;
        case MU_COMMAND_TEXT: drawText(cmd->text.str, cmd->text.pos, cmd->text.color); break;
        case MU_COMMAND_ICON: drawIcon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        case MU_COMMAND_CLIP: g_clip = intersect(cmd->clip.rect, screen); break;
        }
    }
}
