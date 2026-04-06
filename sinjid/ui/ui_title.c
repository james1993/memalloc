#include "ui_internal.h"

void ui_draw_title(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("SINJID", 72)/2, 160, 72, C_GOLD, "SINJID");
    txt(cx - txt_w("Shadow of the Warrior", 28)/2, 248, 28, C_WHITE, "Shadow of the Warrior");
    txt(cx - txt_w("A fan-made simplified clone", 18)/2, 290, 18, C_DIM, "A fan-made simplified clone");

    bool has_save = save_exists();

    if (has_save) {
        Rectangle cont = { cx - 120, 370, 240, 50 };
        if (ui_button(cont, "Continue", true)) {
            if (load_game(&g_ctx.player)) {
                anim_combat_reset();
                anim_fade_to(SCENE_HUB);
            }
        }
    }

    Rectangle btn = { cx - 120, has_save ? 432 : 400, 240, 50 };
    if (ui_button(btn, "New Game", true))
        anim_fade_to(SCENE_CLASS_SELECT);

    if (has_save) {
        Rectangle del = { cx - 70, 496, 140, 34 };
        if (ui_button(del, "Delete Save", true)) {
            save_delete();
            g_ctx.scene_dirty = true;
        }
    }

    txt(cx - txt_w("ESC to quit | F1 debug overlay", 16)/2,
        SCREEN_H - 40, 16, C_DIM, "ESC to quit | F1 debug overlay");
}
