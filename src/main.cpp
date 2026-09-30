#include "ProceduralTextures.h"
#include "Ui.h"
#include <MiniFB.h>
#include <algorithm>
#include <cstring>
#include <vector>

int runChecks();  // Checks.cpp

// Window layout in pixels: texture preview on the left, control panel on the right.
const int TEXTURE_SIZE  = 512;
const int MARGIN        = 16;
const int PANEL_WIDTH   = 300;
const int WINDOW_WIDTH  = MARGIN + TEXTURE_SIZE + MARGIN + PANEL_WIDTH + MARGIN;
const int WINDOW_HEIGHT = MARGIN + TEXTURE_SIZE + MARGIN;
const uint32_t BACKGROUND = 0x202020;

// A button for choosing a texture; the selected texture's button is highlighted.
static bool textureButton(mu_Context* ctx, TextureType type, bool selected) {
    mu_Color normal = ctx->style->colors[MU_COLOR_BUTTON];
    if (selected)
        ctx->style->colors[MU_COLOR_BUTTON] = mu_color(60, 100, 160, 255);
    bool clicked = mu_button(ctx, textureName(type)) != 0;
    ctx->style->colors[MU_COLOR_BUTTON] = normal;
    return clicked;
}

// Editor for one color: a label, a preview swatch and R, G, B sliders (0..255).
static bool colorEditor(mu_Context* ctx, const char* label, Color& color) {
    int labelAndSwatch[] = { 70, -1 };
    mu_layout_row(ctx, 2, labelAndSwatch, 0);
    mu_label(ctx, label);
    mu_draw_rect(ctx, mu_layout_next(ctx), mu_color((int)color.r, (int)color.g, (int)color.b, 255));

    int threeSliders[] = { 94, 94, -1 };
    mu_layout_row(ctx, 3, threeSliders, 0);
    int result = 0;
    result |= mu_slider_ex(ctx, &color.r, 0, 255, 1, "R %.0f", MU_OPT_ALIGNCENTER);
    result |= mu_slider_ex(ctx, &color.g, 0, 255, 1, "G %.0f", MU_OPT_ALIGNCENTER);
    result |= mu_slider_ex(ctx, &color.b, 0, 255, 1, "B %.0f", MU_OPT_ALIGNCENTER);
    return (result & MU_RES_CHANGE) != 0;
}

// Slider for an int value. MicroUI sliders edit floats and derive their id from the float's
// address, so the shared temporary float is combined with an id pushed from the int's address.
static bool intSlider(mu_Context* ctx, int* value, int low, int high) {
    static float tmp;
    mu_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int result = mu_slider_ex(ctx, &tmp, (float)low, (float)high, 1, "%.0f", MU_OPT_ALIGNCENTER);
    *value = (int)tmp;
    mu_pop_id(ctx);
    return (result & MU_RES_CHANGE) != 0;
}

// The control panel on the right. Returns true if a parameter was changed.
static bool controlPanel(mu_Context* ctx, TextureParams& params) {
    mu_Rect area = mu_rect(MARGIN + TEXTURE_SIZE + MARGIN, MARGIN, PANEL_WIDTH, TEXTURE_SIZE);
    if (!mu_begin_window_ex(ctx, "Controls", area, MU_OPT_NOTITLE | MU_OPT_NORESIZE))
        return false;

    bool changed = false;
    int oneColumn[] = { -1 };
    int twoColumns[] = { 143, -1 };

// Texture type: one button per available texture.
    mu_layout_row(ctx, 1, oneColumn, 0);
    mu_label(ctx, "Texture");
    mu_layout_row(ctx, 2, twoColumns, 0);
    for (int i = 0; i < TEX_COUNT; i++) {
        TextureType type = (TextureType)i;
        if (textureButton(ctx, type, type == params.type)) {
            params = defaultParams(type);
            changed = true;
        }
    }

    // The formula of the current texture.
    mu_layout_row(ctx, 1, oneColumn, 0);
    mu_text(ctx, textureFormula(params.type));

    // Parameters of the current texture (later texture types use more of them).
    int labelAndSlider[] = { 70, -1 };
    mu_layout_row(ctx, 2, labelAndSlider, 0);
    if (params.type >= TEX_SINE) {
        mu_label(ctx, "Frequency");
        changed |= (mu_slider_ex(ctx, &params.frequency, 1, 32, 0, "%.1f", MU_OPT_ALIGNCENTER) & MU_RES_CHANGE) != 0;
    }
    if (params.type >= TEX_NOISE) {
        mu_label(ctx, "Seed");
        changed |= intSlider(ctx, &params.seed, 0, 99);
    }
    if (params.type >= TEX_FBM) {
        mu_label(ctx, "Octaves");
        changed |= intSlider(ctx, &params.octaves, 1, 8);
    }
    if (params.type == TEX_MARBLE || params.type == TEX_WOOD) {
        mu_label(ctx, "Distortion");
        changed |= (mu_slider_ex(ctx, &params.distortion, 0, 5, 0, "%.2f", MU_OPT_ALIGNCENTER) & MU_RES_CHANGE) != 0;
    }

    changed |= colorEditor(ctx, "Color A", params.colorA);
    changed |= colorEditor(ctx, "Color B", params.colorB);

    mu_layout_row(ctx, 1, oneColumn, 0);
    if (mu_button(ctx, "Reset")) {
        params = defaultParams(params.type);
        changed = true;
    }

    mu_end_window(ctx);
    return changed;
}

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "--check") == 0)
        return runChecks();

    mfb_window* window = mfb_open_ex("Procedural Texture Toolbox", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window)
        return 1;
    mu_Context* ui = uiInit(window);

    std::vector<uint32_t> frame(WINDOW_WIDTH * WINDOW_HEIGHT);
    std::vector<uint32_t> texture(TEXTURE_SIZE * TEXTURE_SIZE);
    TextureParams params = defaultParams(TEX_GRADIENT);
    bool dirty = true;  // the texture is regenerated only after a parameter changed

    do {
        // 1. UI: MicroUI handles the mouse input and records what to draw.
        mu_begin(ui);
        if (controlPanel(ui, params))
            dirty = true;
        mu_end(ui);

        // 2. Generate the texture if a parameter changed.
        if (dirty) {
            generateTexture(params, texture.data(), TEXTURE_SIZE, TEXTURE_SIZE);
            dirty = false;
        }

        // 3. Compose the frame: background, texture preview, UI on top.
        std::fill(frame.begin(), frame.end(), BACKGROUND);
        for (int y = 0; y < TEXTURE_SIZE; y++)
            std::copy_n(&texture[y * TEXTURE_SIZE], TEXTURE_SIZE, &frame[(MARGIN + y) * WINDOW_WIDTH + MARGIN]);
        uiRender(ui, frame.data(), WINDOW_WIDTH, WINDOW_HEIGHT);

        // 4. Show the frame. MiniFB also delivers the mouse events here.
        if (mfb_update_ex(window, frame.data(), WINDOW_WIDTH, WINDOW_HEIGHT) != MFB_STATE_OK)
            break;
    } while (mfb_wait_sync(window));

    return 0;
}
