#include "ui_internal.h"

static bool s_show_skills = false;
static bool s_show_items  = false;

static void draw_combatant(int x, int y, int w, const char *name,
    float disp_life, float disp_mana, int max_life, int max_mana,
    Color body_col, StatusFlags status, float bob_t, float flash, bool heal_flash)
{
    int bob = (int)anim_bob_y(bob_t);
    y += bob;

    DrawRectangle(x, y, w, 120, body_col);
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,120}, 2, C_BORDER);

    if (flash > 0.0f) {
        Color fc = heal_flash ? (Color){60,220,60,255} : (Color){255,255,255,255};
        fc.a = (unsigned char)(flash * 180);
        DrawRectangle(x, y, w, 120, fc);
    }

    txt(x + 8, y + 8, 20, C_WHITE, name);

    txt(x + 8, y + 38, 13, C_DIM, "HP");
    ui_bar_f(x + 30, y + 38, w - 40, 13, disp_life, max_life, C_HP, C_BTN);
    txt(x + 8, y + 55, 12, C_DIM, TextFormat("%d/%d", (int)disp_life, max_life));

    txt(x + 8, y + 72, 13, C_DIM, "MP");
    ui_bar_f(x + 30, y + 72, w - 40, 13, disp_mana, max_mana, C_MP, C_BTN);
    txt(x + 8, y + 89, 12, C_DIM, TextFormat("%d/%d", (int)disp_mana, max_mana));

    int sx = x + 8, sy = y + 108;
    if (status & STATUS_POISONED) { txt(sx, sy, 11, C_GREEN, "PSN"); sx += 28; }
    if (status & STATUS_STUNNED)  { txt(sx, sy, 11, C_GOLD,  "STN"); sx += 28; }
    if (status & STATUS_SLOWED)   { txt(sx, sy, 11, C_MP,    "SLW"); sx += 28; }
    if (status & STATUS_POWERED)  { txt(sx, sy, 11, C_RED,   "PWR"); sx += 28; }
    if (status & STATUS_SHIELDED) { txt(sx, sy, 11, C_WHITE, "SHD"); sx += 28; }
    if (status & STATUS_DODGING)  { txt(sx, sy, 11, C_XP,    "DDG"); sx += 28; }
    (void)sx;
}

/* Drain the event queue and fire audio/anim reactions */
static void process_events(void)
{
    Event ev;
    while (evt_pop(&g_ctx.events, &ev)) {
        switch (ev.type) {
            case EVT_HIT_ENEMY:   anim_enemy_hit(false);  snd_hit();   break;
            case EVT_HIT_PLAYER:  anim_player_hit(false); snd_hit();   break;
            case EVT_HEAL_PLAYER: anim_player_hit(true);               break;
            case EVT_SKILL_USE:                            snd_skill(); break;
            case EVT_LEVEL_UP:                             snd_level_up(); break;
            default: break;
        }
    }
}

void ui_draw_combat(void)
{
    CombatState *cs = &g_ctx.combat;

    /* Drain events pushed by update.c (enemy turn) or player actions */
    process_events();

    draw_combatant(30, 30, 300, g_ctx.player.name,
        g_ctx.canim.player_life, g_ctx.canim.player_mana,
        player_effective_life(&g_ctx.player), player_effective_mana(&g_ctx.player),
        (Color){40,60,120,255}, g_ctx.player.status,
        g_ctx.canim.player_bob_t, g_ctx.canim.player_flash, g_ctx.canim.player_flash_col);

    draw_combatant(SCREEN_W - 330, 30, 300, cs->enemy.name,
        g_ctx.canim.enemy_life, g_ctx.canim.enemy_mana,
        cs->enemy.base_stats.max_life, cs->enemy.base_stats.max_mana,
        cs->enemy.color, cs->enemy.status,
        g_ctx.canim.enemy_bob_t, g_ctx.canim.enemy_flash, g_ctx.canim.enemy_flash_col);

    const char *turn_txt = cs->combat_over ? "" :
        (cs->player_turn ? ">> Your turn <<" : "Enemy acting...");
    txt(SCREEN_W/2 - txt_w(turn_txt, 20)/2, 60, 20, C_GOLD, turn_txt);

    Rectangle log_box = { 340, 20, SCREEN_W - 680, 380 };
    ui_panel(log_box, C_PANEL, C_BORDER);
    txt((int)log_box.x + 8, (int)log_box.y + 6, 13, C_DIM, "Combat Log");
    int visible = cs->log_count < 7 ? cs->log_count : 7;
    for (int i = 0; i < visible; i++) {
        int li = cs->log_count - visible + i;
        txt((int)log_box.x + 8,
            (int)log_box.y + 26 + i * 50,
            15, (i == visible - 1) ? C_WHITE : C_DIM,
            cs->log[li]);
    }

    if (cs->player_turn && !cs->combat_over) {
        int by = 430;
        if (!s_show_skills && !s_show_items) {
            Rectangle ra = {  30, (float)by, 180, 48 };
            Rectangle rs = { 220, (float)by, 180, 48 };
            Rectangle ri = { 410, (float)by, 180, 48 };
            Rectangle rf = { 600, (float)by, 180, 48 };

            if (ui_button(ra, "Attack", true)) {
                combat_action_attack();
                process_events();
                s_show_skills = false; s_show_items = false;
            }
            if (ui_button(rs, "Skills", true)) { s_show_skills = true; s_show_items = false; }
            if (ui_button(ri, "Items",  true)) { s_show_items  = true; s_show_skills = false; }
            if (ui_button(rf, "Flee",   true)) { combat_action_flee(); }
        }

        if (s_show_skills) {
            txt(30, (float)(by - 26), 17, C_GOLD, "Choose a skill:");
            const SkillDef *skills = skills_for_class(g_ctx.player.pc);
            for (int i = 0; i < MAX_SKILLS; i++) {
                const SkillDef *sk = &skills[i];
                int slvl = g_ctx.player.skill_level[i];
                int cost = sk->mana_cost + (slvl - 1) * 2;
                bool can = (g_ctx.player.current_mana >= cost);
                char lbl[80];
                snprintf(lbl, sizeof(lbl), "%s (Lv%d) [%dMP]", sk->name, slvl, cost);
                Rectangle sb = { 30, (float)(by + i * 56), 600, 48 };
                if (ui_button(sb, lbl, can)) {
                    combat_action_skill(i);
                    process_events();
                    s_show_skills = false;
                }
            }
            Rectangle cancel = { 650, (float)by, 140, 48 };
            if (ui_button(cancel, "Cancel", true)) s_show_skills = false;
        }

        if (s_show_items) {
            txt(30, (float)(by - 26), 17, C_GOLD, "Use an item:");
            int shown = 0;
            for (int i = 0; i < MAX_INVENTORY && shown < 6; i++) {
                int id = g_ctx.player.bag_ids[i];
                if (id < 0 || g_items[id].type != ITEM_CONSUMABLE) continue;
                char lbl[64];
                snprintf(lbl, sizeof(lbl), "%s x%d", g_items[id].name, g_ctx.player.bag_qty[i]);
                Rectangle ib = { 30, (float)(by + shown * 56), 400, 48 };
                if (ui_button(ib, lbl, true)) {
                    combat_action_use_item(id);
                    process_events();
                    s_show_items = false;
                }
                shown++;
            }
            if (shown == 0) txt(30, (float)by, 17, C_DIM, "No consumables.");
            Rectangle cancel2 = { 450, (float)by, 140, 48 };
            if (ui_button(cancel2, "Cancel", true)) s_show_items = false;
        }
    }

    if (cs->combat_over) {
        s_show_skills = false; s_show_items = false;
        Rectangle overlay = { 300, 280, 424, 220 };
        ui_panel(overlay, (Color){10,10,20,230}, cs->player_won ? C_XP : C_RED);

        if (cs->fled) {
            txt_c((Rectangle){300,300,424,40}, 22, C_GOLD, "You fled the battle!");
            Rectangle rb = { 380, 400, 260, 50 };
            if (ui_button(rb, "Return to Temple", true)) {
                g_ctx.scene_dirty = true;
                anim_fade_to(SCENE_HUB);
            }
        } else if (cs->player_won) {
            txt_c((Rectangle){300,300,424,40}, 28, C_XP, "Victory!");
            txt(340, 346, 17, C_GOLD,
                TextFormat("+%d XP   +%d Gold",
                    cs->enemy.xp_reward, cs->enemy.gold_reward));
            if (g_ctx.player.gw_complete[cs->gateway])
                txt(340, 372, 19, C_GREEN, "Gateway Complete!");

            Rectangle rb = { 380, 418, 260, 50 };
            if (ui_button(rb, "Continue", true)) {
                player_rest(&g_ctx.player);
                g_ctx.scene_dirty = true;
                if (g_ctx.player.stat_points > 0 || g_ctx.player.skill_points > 0) {
                    snd_level_up();
                    anim_fade_to(SCENE_LEVEL_UP);
                } else if (g_ctx.player.gw_complete[GW_DARK_RIFT]) {
                    snd_victory();
                    anim_fade_to(SCENE_VICTORY);
                } else {
                    anim_fade_to(SCENE_HUB);
                }
            }
        } else {
            txt_c((Rectangle){300,300,424,40}, 28, C_RED, "Defeated...");
            Rectangle retry = { 320, 398, 180, 50 };
            Rectangle give  = { 520, 398, 180, 50 };
            if (ui_button(retry, "Retry", true)) {
                combat_start(cs->gateway, cs->floor);
                anim_combat_reset();
                player_rest(&g_ctx.player);
                g_ctx.scene_dirty = true;
            }
            if (ui_button(give, "Give Up", true)) {
                snd_game_over();
                anim_fade_to(SCENE_GAME_OVER);
            }
        }
    }
}
