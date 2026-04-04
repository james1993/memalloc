#include "ui_internal.h"

void ui_draw_level_up(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("LEVEL UP!", 46)/2, 60, 46, C_GOLD, "LEVEL UP!");
    txt(cx - txt_w(TextFormat("You are now Level %d!", g_ctx.player.level), 22)/2,
        118, 22, C_WHITE, TextFormat("You are now Level %d!", g_ctx.player.level));
    txt(cx - 160, 160, 19, C_WHITE, TextFormat("Stat points:  %d", g_ctx.player.stat_points));
    txt(cx - 160, 184, 17, C_DIM,   TextFormat("Skill points: %d", g_ctx.player.skill_points));

    static const struct { const char *lbl; int field; int delta; } stats[] = {
        { "+Life (+10)",    0, 10 },
        { "+Mana (+10)",    1, 10 },
        { "+Strength (+2)", 2,  2 },
        { "+Speed (+2)",    3,  2 },
        { "+Defense (+1)",  4,  1 },
    };
    for (int i = 0; i < 5; i++) {
        Rectangle btn = { (float)(cx - 150), (float)(230 + i * 60), 300, 48 };
        bool can = (g_ctx.player.stat_points > 0);
        if (ui_button(btn, stats[i].lbl, can) && can) {
            g_ctx.player.stat_points--;
            int *fields[] = {
                &g_ctx.player.base.max_life, &g_ctx.player.base.max_mana,
                &g_ctx.player.base.strength, &g_ctx.player.base.speed,
                &g_ctx.player.base.defense
            };
            *fields[stats[i].field] += stats[i].delta;
            if (stats[i].field == 0) g_ctx.player.current_life += stats[i].delta;
            if (stats[i].field == 1) g_ctx.player.current_mana += stats[i].delta;
            g_ctx.scene_dirty = true;
        }
    }

    Rectangle done_btn = { (float)(cx - 100), 556, 200, 50 };
    bool done = (g_ctx.player.stat_points == 0);
    if (ui_button(done_btn, done ? "Done!" : "Skip for now", true)) {
        if (g_ctx.player.skill_points > 0) anim_fade_to(SCENE_SKILLS);
        else anim_fade_to(SCENE_HUB);
    }
}
