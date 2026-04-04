#include "ui_internal.h"

void ui_draw_gateway_select(void)
{
    GatewayId gw = g_ctx.combat.gateway;
    static const char *gw_labels[] = { "Human Gateway", "Monster Portal", "Dark Rift" };
    static const char *gw_flavor[] = {
        "5 floors of human warriors - main story path.",
        "5 floors of monsters - optional challenge.",
        "5 floors of darkness - post-game ultimate challenge."
    };
    int cx = SCREEN_W / 2;
    txt(cx - txt_w(gw_labels[gw], 30)/2, 30, 30, C_GOLD, gw_labels[gw]);
    txt(cx - txt_w(gw_flavor[gw], 15)/2, 72, 15, C_DIM, gw_flavor[gw]);

    int prog = g_ctx.player.gw_progress[gw];
    txt(cx - 120, 100, 17, C_WHITE,
        TextFormat("Progress: %d / %d floors", prog, GATEWAY_DEPTH));

    for (int f = 0; f < GATEWAY_DEPTH; f++) {
        int eid = g_gw_enemies[gw][f];
        const EnemyDef *e = &g_enemies[eid];
        bool completed = (f < prog);
        bool current   = (f == prog);

        Rectangle row = { (float)(cx - 260), (float)(144 + f * 88), 520, 76 };
        Color border = completed ? C_GREEN : (current ? C_GOLD : C_BORDER);
        ui_panel(row, C_PANEL, border);

        txt(cx - 244, (int)row.y + 10, 19,
            completed ? C_GREEN : (current ? C_WHITE : C_DIM),
            TextFormat("Floor %d: %s", f + 1, e->name));
        txt(cx - 244, (int)row.y + 38, 13, C_DIM,
            TextFormat("HP: %d  STR: %d  SPD: %d  XP: %d  Gold: %d",
                e->base_stats.max_life, e->base_stats.strength,
                e->base_stats.speed, e->xp_reward, e->gold_reward));

        if (completed) {
            txt((int)row.x + (int)row.width - 100, (int)row.y + 26, 17, C_GREEN, "CLEARED");
        } else if (current) {
            Rectangle fb = { row.x + row.width - 140, row.y + 14, 130, 44 };
            if (ui_button(fb, "Enter!", true)) {
                combat_start(gw, f);
                anim_combat_reset();
                scene_replace(&g_ctx.scenes, SCENE_COMBAT);
                g_ctx.scene_dirty = true;
            }
        } else {
            txt((int)row.x + (int)row.width - 90, (int)row.y + 26, 17, C_DIM, "LOCKED");
        }
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) anim_fade_to(SCENE_HUB);
}
