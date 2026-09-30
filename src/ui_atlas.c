/* MicroUI's built-in font and icons. atlas.inl uses C99 array initializers
   ("[index] = value"), which C++ does not support, so this file is compiled as C. */
#include "microui.h"
#include "atlas.inl"

/* Alpha (coverage, 0..255) of the atlas image at pixel (x, y). */
unsigned char ui_atlas_alpha(int x, int y) {
    return atlas_texture[y * ATLAS_WIDTH + x];
}

/* Where a character is located in the atlas image (the font covers ASCII 32..127). */
mu_Rect ui_atlas_char(unsigned char c) {
    return atlas[ATLAS_FONT + (c > 127 ? 127 : c)];
}

/* Where an icon (MU_ICON_CLOSE, MU_ICON_CHECK, ...) is located in the atlas image. */
mu_Rect ui_atlas_icon(int id) {
    return atlas[id];
}
