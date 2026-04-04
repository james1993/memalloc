#include "ui_internal.h"

/* s_name_buf/s_naming/s_name_len are owned by ui_classselect.c */
extern char s_name_buf[];
extern int  s_name_len;
extern bool s_naming;

void ui_draw_victory(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("VICTORY!", 58)/2, 100, 58, C_GOLD, "VICTORY!");
    txt(cx - txt_w("You have conquered the Dark Rift!", 22)/2,
        174, 22, C_WHITE, "You have conquered the Dark Rift!");
    txt(cx - txt_w("Warlord Baka has been defeated.", 15)/2,
        210, 15, C_DIM, "Warlord Baka has been defeated.");
    txt(cx - 100, 268, 19, C_WHITE, TextFormat("Final Level: %d", g_ctx.player.level));
    txt(cx - 100, 294, 19, C_GOLD,  TextFormat("Gold: %d",        g_ctx.player.gold));

    Rectangle hub_btn  = { (float)(cx - 230), 378, 200, 54 };
    Rectangle play_btn = { (float)(cx + 30),  378, 200, 54 };
    if (ui_button(hub_btn, "Return to Hub", true)) anim_fade_to(SCENE_HUB);
    if (ui_button(play_btn,"Play Again",    true)) {
        g_ctx.scene_dirty = true;
        s_naming  = false;
        s_name_len = 4;
        memcpy(s_name_buf, "Hero", 5);
        anim_fade_to(SCENE_TITLE);
    }
}
