#include "ui_internal.h"

void ui_draw_hub(void)
{
    Rectangle left = { 20, 20, 420, SCREEN_H - 40 };
    ui_panel(left, C_PANEL, C_BORDER);
    txt(36, 34, 24, C_GOLD, "Shadow Temple");

    static const HubLocation locs[] = {
        HUB_GW_HUMAN, HUB_GW_MONSTER, HUB_GW_DARK,
        HUB_TRAINING,
        HUB_SHOP_BASIC, HUB_SHOP_MID, HUB_SHOP_ADV,
        HUB_SKILLS, HUB_INVENTORY
    };
    for (int i = 0; i < 9; i++) {
        HubLocation loc = locs[i];
        bool unlocked   = hub_location_unlocked(loc);
        Rectangle btn   = { 36, 80 + i * 60, 388, 48 };
        char buf[64];
        if (!unlocked) snprintf(buf, sizeof(buf), "[LOCKED] %s", hub_location_name(loc));
        else           snprintf(buf, sizeof(buf), "%s", hub_location_name(loc));
        if (ui_button(btn, buf, unlocked)) hub_enter(loc);
    }

    Rectangle right = { 460, 20, SCREEN_W - 480, SCREEN_H - 40 };
    ui_panel(right, C_PANEL, C_BORDER);

    int rx = 474, ry = 34;
    txt(rx, ry,      22, C_GOLD,  TextFormat("%s the %s", g_ctx.player.name, class_name(g_ctx.player.pc)));
    txt(rx, ry + 30, 18, C_WHITE, TextFormat("Level %d", g_ctx.player.level));
    txt(rx, ry + 52, 18, C_GOLD,  TextFormat("Gold: %d", g_ctx.player.gold));

    txt(rx, ry + 80, 15, C_DIM, "XP:");
    ui_bar(rx + 28, ry + 80, 200, 13,
           g_ctx.player.xp, g_ctx.player.xp_to_next, C_XP, C_BTN);
    txt(rx + 232, ry + 80, 13, C_DIM,
        TextFormat("%d / %d", g_ctx.player.xp, g_ctx.player.xp_to_next));

    int max_life = player_effective_life();
    int max_mana = player_effective_mana();
    txt(rx, ry + 102, 15, C_DIM, "HP:");
    ui_bar(rx + 28, ry + 102, 200, 13,
           g_ctx.player.current_life, max_life, C_HP, C_BTN);
    txt(rx + 232, ry + 102, 13, C_DIM,
        TextFormat("%d/%d", g_ctx.player.current_life, max_life));

    txt(rx, ry + 122, 15, C_DIM, "MP:");
    ui_bar(rx + 28, ry + 122, 200, 13,
           g_ctx.player.current_mana, max_mana, C_MP, C_BTN);
    txt(rx + 232, ry + 122, 13, C_DIM,
        TextFormat("%d/%d", g_ctx.player.current_mana, max_mana));

    txt(rx, ry + 150, 15, C_DIM, "-- Stats --");
    txt(rx, ry + 170, 16, C_WHITE, TextFormat("Strength:  %d", player_effective_str()));
    txt(rx, ry + 190, 16, C_WHITE, TextFormat("Speed:     %d", player_effective_spd()));
    txt(rx, ry + 210, 16, C_WHITE, TextFormat("Defense:   %d", player_effective_def()));

    txt(rx, ry + 242, 15, C_DIM, "-- Equipment --");
    static const char *slot_names[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_ctx.player.equip[s];
        txt(rx, ry + 262 + s * 22, 15,
            (eid >= 0) ? C_WHITE : C_DIM,
            TextFormat("%s: %s", slot_names[s],
                       (eid >= 0) ? g_items[eid].name : "(none)"));
    }

    txt(rx, ry + 344, 15, C_DIM, "-- Gateways --");
    static const char *gw_names[] = { "Human Gateway", "Monster Portal", "Dark Rift" };
    for (int g = 0; g < NUM_GATEWAYS; g++) {
        int  prog = g_ctx.player.gw_progress[g];
        bool comp = g_ctx.player.gw_complete[g];
        Color gc  = comp ? C_GREEN : (prog > 0 ? C_GOLD : C_DIM);
        txt(rx, ry + 364 + g * 22, 15, gc,
            TextFormat("%s: %s%d/%d", gw_names[g],
                       comp ? "COMPLETE " : "", prog, GATEWAY_DEPTH));
    }

    if (g_ctx.player.stat_points > 0 || g_ctx.player.skill_points > 0) {
        txt(rx, ry + 440, 15, C_GOLD,
            TextFormat("! Points available (%d stat, %d skill)",
                g_ctx.player.stat_points, g_ctx.player.skill_points));
        Rectangle lub = { (float)rx, (float)(ry + 462), 220, 38 };
        if (ui_button(lub, "Allocate Points", true))
            anim_fade_to(SCENE_LEVEL_UP);
    }

    static float save_flash = 0.0f;
    save_flash -= GetFrameTime();
    Rectangle save_btn = { (float)rx, (float)(SCREEN_H - 56), 140, 38 };
    if (ui_button(save_btn, "Save Game", true)) {
        save_game(&g_ctx.player);
        save_flash = 2.0f;
        g_ctx.scene_dirty = true;
    }
    if (save_flash > 0.0f)
        txt(rx + 150, SCREEN_H - 48, 15, C_GREEN, "Saved!");
}
