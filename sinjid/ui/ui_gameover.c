#include "ui_internal.h"

/* s_name_buf/s_naming/s_name_len are owned by ui_classselect.c */
extern char s_name_buf[];
extern int  s_name_len;
extern bool s_naming;

void ui_draw_game_over(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("GAME OVER", 58)/2, 160, 58, C_RED, "GAME OVER");
    txt(cx - 200, 248, 21, C_WHITE,
        TextFormat("You fell as a Level %d %s.", g_ctx.player.level, class_name(g_ctx.player.pc)));
    txt(cx - 80, 282, 17, C_GOLD, TextFormat("Gold earned: %d", g_ctx.player.gold));

    Rectangle replay = { (float)(cx - 110), 358, 220, 54 };
    if (ui_button(replay, "Play Again", true)) {
        g_ctx.scene_dirty = true;
        s_naming  = false;
        s_name_len = 4;
        memcpy(s_name_buf, "Hero", 5);
        anim_fade_to(SCENE_TITLE);
    }
}
