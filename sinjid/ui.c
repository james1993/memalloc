#include "ui.h"
#include "game.h"
#include "combat.h"
#include "player.h"
#include "hub.h"
#include "data.h"
#include "anim.h"
#include "audio.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ── Font ── */
static Font g_font;
static bool g_font_loaded = false;

void ui_init(void)
{
    const char *ttf = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
    if (FileExists(ttf)) {
        g_font = LoadFontEx(ttf, 20, NULL, 0);
        g_font_loaded = true;
    }
}

void ui_close(void)
{
    if (g_font_loaded) UnloadFont(g_font);
}

/* Draw text using loaded font, falling back to default */
static void txt(int x, int y, int size, Color col, const char *s)
{
    if (g_font_loaded)
        DrawTextEx(g_font, s, (Vector2){(float)x,(float)y},
                   (float)size, 1.0f, col);
    else
        DrawText(s, x, y, size, col);
}

static int txt_w(const char *s, int size)
{
    if (g_font_loaded)
        return (int)MeasureTextEx(g_font, s, (float)size, 1.0f).x;
    return MeasureText(s, size);
}

/* Centered text in a rect */
static void txt_c(Rectangle r, int size, Color col, const char *s)
{
    int tw = txt_w(s, size);
    int tx = (int)(r.x + (r.width  - tw)   / 2);
    int ty = (int)(r.y + (r.height - size)  / 2);
    txt(tx, ty, size, col, s);
}

/* ── Palette ── */
static const Color C_PANEL   = {  28,  28,  45, 255 };
static const Color C_BORDER  = {  70,  70, 120, 255 };
static const Color C_GOLD    = { 220, 180,  40, 255 };
static const Color C_WHITE   = { 230, 230, 230, 255 };
static const Color C_DIM     = { 100, 100, 130, 255 };
static const Color C_BTN     = {  45,  45,  80, 255 };
static const Color C_BTN_HOV = {  70,  70, 130, 255 };
static const Color C_BTN_DIS = {  30,  30,  45, 255 };
static const Color C_HP      = { 200,  40,  40, 255 };
static const Color C_MP      = {  40,  80, 200, 255 };
static const Color C_XP      = {  40, 180,  80, 255 };
static const Color C_GREEN   = {  60, 200,  60, 255 };
static const Color C_RED     = { 220,  60,  60, 255 };

/* ── Helpers ── */
void ui_panel(Rectangle r, Color fill, Color border)
{
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, border);
}

void ui_bar(int x, int y, int w, int h, int cur, int max, Color fill, Color bg)
{
    DrawRectangle(x, y, w, h, bg);
    if (max > 0) {
        int filled = (int)((float)cur / (float)max * w);
        if (filled > w) filled = w;
        if (filled > 0) DrawRectangle(x, y, filled, h, fill);
    }
    DrawRectangleLines(x, y, w, h, C_BORDER);
}

/* Animated bar: uses float display value */
static void ui_bar_f(int x, int y, int w, int h,
                     float cur, int max, Color fill, Color bg)
{
    DrawRectangle(x, y, w, h, bg);
    if (max > 0 && cur > 0) {
        int filled = (int)(cur / (float)max * w);
        if (filled > w) filled = w;
        if (filled > 0) DrawRectangle(x, y, filled, h, fill);
    }
    DrawRectangleLines(x, y, w, h, C_BORDER);
}

void ui_text_center(Rectangle r, const char *text, int font_size, Color col)
{
    txt_c(r, font_size, col, text);
}

/* Button: returns true on click. Plays sounds on hover/click. */

bool ui_button(Rectangle r, const char *label, bool enabled)
{
    Vector2 mouse = GetMousePosition();
    bool hovered  = enabled && CheckCollisionPointRec(mouse, r);
    Color bg      = !enabled ? C_BTN_DIS : (hovered ? C_BTN_HOV : C_BTN);
    Color tc      = !enabled ? C_DIM     : C_WHITE;
    ui_panel(r, bg, C_BORDER);
    txt_c(r, 18, tc, label);

    static Rectangle last_hovered = {0};
    if (hovered && (last_hovered.x != r.x || last_hovered.y != r.y)) {
        snd_hover();
        last_hovered = r;
    }
    if (!hovered && last_hovered.x == r.x && last_hovered.y == r.y)
        last_hovered = (Rectangle){0};

    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (clicked) snd_click();
    return clicked;
}

/* ── Name-input state ── */
static char s_name_buf[32] = "Hero";
static int  s_name_len     = 4;
static bool s_naming       = false;
static PlayerClass s_chosen_class = CLASS_WARRIOR;

/* ── TITLE ── */
void ui_draw_title(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("SINJID", 72)/2, 160, 72, C_GOLD, "SINJID");
    txt(cx - txt_w("Shadow of the Warrior", 28)/2, 248, 28, C_WHITE, "Shadow of the Warrior");
    txt(cx - txt_w("A fan-made simplified clone", 18)/2, 290, 18, C_DIM, "A fan-made simplified clone");

    Rectangle btn = { cx - 120, 400, 240, 50 };
    if (ui_button(btn, "Begin Your Journey", true))
        anim_fade_to(SCENE_CLASS_SELECT);

    txt(cx - txt_w("ESC to quit | F1 debug overlay", 16)/2,
        SCREEN_H - 40, 16, C_DIM, "ESC to quit | F1 debug overlay");
}

/* ── CLASS SELECT ── */
void ui_draw_class_select(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("Choose Your Class", 32)/2, 40, 32, C_GOLD, "Choose Your Class");

    static const struct {
        PlayerClass pc; const char *name; const char *desc; Color col;
    } classes[] = {
        { CLASS_WARRIOR, "Warrior", "High STR & DEF. Buffs and stuns.", (Color){180,80,80,255} },
        { CLASS_ROGUE,   "Rogue",   "High SPD. Poison, dodge, multi-hit.", (Color){80,180,80,255} },
        { CLASS_MAGE,    "Mage",    "High mana. Devastating spells.", (Color){80,80,200,255} },
    };

    if (!s_naming) {
        for (int i = 0; i < 3; i++) {
            Rectangle box = { 80 + i * 300, 120, 260, 200 };
            bool sel = (s_chosen_class == classes[i].pc);
            ui_panel(box, sel ? (Color){40,40,80,255} : C_PANEL,
                     sel ? C_GOLD : C_BORDER);
            DrawRectangle((int)box.x + 10, (int)box.y + 10, 40, 40, classes[i].col);
            txt((int)box.x + 60, (int)box.y + 18, 22, C_WHITE, classes[i].name);

            Stats st = class_base_stats(classes[i].pc);
            txt((int)box.x+14, (int)box.y+70,  16, C_HP,    TextFormat("Life:  %d", st.max_life));
            txt((int)box.x+14, (int)box.y+90,  16, C_MP,    TextFormat("Mana:  %d", st.max_mana));
            txt((int)box.x+14, (int)box.y+110, 16, C_WHITE, TextFormat("STR:   %d", st.strength));
            txt((int)box.x+14, (int)box.y+130, 16, C_WHITE, TextFormat("SPD:   %d", st.speed));
            txt((int)box.x+14, (int)box.y+150, 16, C_WHITE, TextFormat("DEF:   %d", st.defense));

            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                s_chosen_class = classes[i].pc;
                snd_click();
            }
        }

        txt(80, 340, 15, C_DIM, "Warrior: tanky melee fighter with buffs and stuns");
        txt(80, 360, 15, C_DIM, "Rogue:   fast striker with poison, dodge, and multi-hit");
        txt(80, 380, 15, C_DIM, "Mage:    fragile caster with high-damage spells and heal");

        Rectangle next = { cx - 100, 440, 200, 48 };
        if (ui_button(next, "Choose Name ->", true)) s_naming = true;

        Rectangle back = { cx - 100, 500, 200, 40 };
        if (ui_button(back, "<- Back", true)) anim_fade_to(SCENE_TITLE);
    } else {
        txt(cx - txt_w("Enter your name:", 24)/2, 200, 24, C_WHITE, "Enter your name:");
        Rectangle nbox = { cx - 160, 250, 320, 48 };
        ui_panel(nbox, C_PANEL, C_GOLD);
        txt((int)nbox.x + 12, (int)nbox.y + 14, 22, C_WHITE, s_name_buf);
        if (((int)(GetTime() * 2)) % 2 == 0)
            txt((int)nbox.x + 12 + txt_w(s_name_buf, 22),
                (int)nbox.y + 14, 22, C_GOLD, "_");

        int ch;
        while ((ch = GetCharPressed()) != 0) {
            if (ch >= 32 && s_name_len < 30) {
                s_name_buf[s_name_len++] = (char)ch;
                s_name_buf[s_name_len]   = '\0';
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE) && s_name_len > 0)
            s_name_buf[--s_name_len] = '\0';

        bool name_ok = s_name_len > 0;
        Rectangle start = { cx - 100, 340, 200, 48 };
        if (ui_button(start, "Start Game!", name_ok) && name_ok) {
            player_init(&g_player, s_name_buf, s_chosen_class);
            anim_combat_reset();
            s_naming = false;
            anim_fade_to(SCENE_HUB);
        }
        Rectangle back2 = { cx - 100, 400, 200, 40 };
        if (ui_button(back2, "<- Back", true)) s_naming = false;
    }
}

/* ── HUB ── */
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
    txt(rx, ry,      22, C_GOLD,  TextFormat("%s the %s", g_player.name, class_name(g_player.pc)));
    txt(rx, ry + 30, 18, C_WHITE, TextFormat("Level %d", g_player.level));
    txt(rx, ry + 52, 18, C_GOLD,  TextFormat("Gold: %d", g_player.gold));

    txt(rx, ry + 80, 15, C_DIM, "XP:");
    ui_bar(rx + 28, ry + 80, 200, 13,
           g_player.xp, g_player.xp_to_next, C_XP, C_BTN);
    txt(rx + 232, ry + 80, 13, C_DIM,
        TextFormat("%d / %d", g_player.xp, g_player.xp_to_next));

    int max_life = player_effective_life();
    int max_mana = player_effective_mana();
    txt(rx, ry + 102, 15, C_DIM, "HP:");
    ui_bar(rx + 28, ry + 102, 200, 13,
           g_player.current_life, max_life, C_HP, C_BTN);
    txt(rx + 232, ry + 102, 13, C_DIM,
        TextFormat("%d/%d", g_player.current_life, max_life));

    txt(rx, ry + 122, 15, C_DIM, "MP:");
    ui_bar(rx + 28, ry + 122, 200, 13,
           g_player.current_mana, max_mana, C_MP, C_BTN);
    txt(rx + 232, ry + 122, 13, C_DIM,
        TextFormat("%d/%d", g_player.current_mana, max_mana));

    txt(rx, ry + 150, 15, C_DIM, "-- Stats --");
    txt(rx, ry + 170, 16, C_WHITE, TextFormat("Strength:  %d", player_effective_str()));
    txt(rx, ry + 190, 16, C_WHITE, TextFormat("Speed:     %d", player_effective_spd()));
    txt(rx, ry + 210, 16, C_WHITE, TextFormat("Defense:   %d", player_effective_def()));

    txt(rx, ry + 242, 15, C_DIM, "-- Equipment --");
    static const char *slot_names[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_player.equip[s];
        txt(rx, ry + 262 + s * 22, 15,
            (eid >= 0) ? C_WHITE : C_DIM,
            TextFormat("%s: %s", slot_names[s],
                       (eid >= 0) ? g_items[eid].name : "(none)"));
    }

    txt(rx, ry + 344, 15, C_DIM, "-- Gateways --");
    static const char *gw_names[] = { "Human Gateway", "Monster Portal", "Dark Rift" };
    for (int g = 0; g < NUM_GATEWAYS; g++) {
        int  prog = g_player.gw_progress[g];
        bool comp = g_player.gw_complete[g];
        Color gc  = comp ? C_GREEN : (prog > 0 ? C_GOLD : C_DIM);
        txt(rx, ry + 364 + g * 22, 15, gc,
            TextFormat("%s: %s%d/%d", gw_names[g],
                       comp ? "COMPLETE " : "", prog, GATEWAY_DEPTH));
    }

    if (g_player.stat_points > 0 || g_player.skill_points > 0) {
        txt(rx, ry + 440, 15, C_GOLD,
            TextFormat("! Points available (%d stat, %d skill)",
                g_player.stat_points, g_player.skill_points));
        Rectangle lub = { (float)rx, (float)(ry + 462), 220, 38 };
        if (ui_button(lub, "Allocate Points", true))
            anim_fade_to(SCENE_LEVEL_UP);
    }
}

/* ── COMBAT ── */
static bool s_show_skills = false;
static bool s_show_items  = false;

/* Draw combatant card with idle bob and hit flash */
static void draw_combatant(int x, int y, int w, const char *name,
    float disp_life, float disp_mana, int max_life, int max_mana,
    Color body_col, StatusFlags status, float bob_t, float flash, bool heal_flash)
{
    int bob = (int)anim_bob_y(bob_t);
    y += bob;

    DrawRectangle(x, y, w, 120, body_col);
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,120}, 2, C_BORDER);

    /* Hit flash overlay */
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

void ui_draw_combat(void)
{
    CombatState *cs = &g_combat;

    /* Auto-advance enemy turn with a brief delay so log is readable */
    if (!cs->player_turn && !cs->combat_over) {
        static float enemy_delay = 0.6f;
        enemy_delay -= GetFrameTime();
        if (enemy_delay <= 0.0f) {
            int prev_life = g_player.current_life;
            combat_enemy_turn();
            if (g_player.current_life < prev_life) {
                anim_player_hit(false);
                snd_hit();
            }
            enemy_delay = 0.6f;
        }
    }

    /* Combatant cards */
    draw_combatant(30, 30, 300, g_player.name,
        g_canim.player_life, g_canim.player_mana,
        player_effective_life(), player_effective_mana(),
        (Color){40,60,120,255}, g_player.status,
        g_canim.player_bob_t, g_canim.player_flash, g_canim.player_flash_col);

    draw_combatant(SCREEN_W - 330, 30, 300, cs->enemy.name,
        g_canim.enemy_life, g_canim.enemy_mana,
        cs->enemy.base_stats.max_life, cs->enemy.base_stats.max_mana,
        cs->enemy.color, cs->enemy.status,
        g_canim.enemy_bob_t, g_canim.enemy_flash, g_canim.enemy_flash_col);

    const char *turn_txt = cs->combat_over ? "" :
        (cs->player_turn ? ">> Your turn <<" : "Enemy acting...");
    txt(SCREEN_W/2 - txt_w(turn_txt, 20)/2, 60, 20, C_GOLD, turn_txt);

    /* Combat log */
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

    /* Action bar */
    if (cs->player_turn && !cs->combat_over) {
        int by = 430;
        if (!s_show_skills && !s_show_items) {
            Rectangle ra = {  30, (float)by, 180, 48 };
            Rectangle rs = { 220, (float)by, 180, 48 };
            Rectangle ri = { 410, (float)by, 180, 48 };
            Rectangle rf = { 600, (float)by, 180, 48 };

            if (ui_button(ra, "Attack", true)) {
                int prev = cs->enemy.current_life;
                combat_action_attack();
                if (cs->enemy.current_life < prev) {
                    anim_enemy_hit(false); snd_hit();
                }
                s_show_skills = false; s_show_items = false;
            }
            if (ui_button(rs, "Skills", true)) { s_show_skills = true; s_show_items = false; }
            if (ui_button(ri, "Items",  true)) { s_show_items  = true; s_show_skills = false; }
            if (ui_button(rf, "Flee",   true)) { combat_action_flee(); }
        }

        if (s_show_skills) {
            txt(30, (float)(by - 26), 17, C_GOLD, "Choose a skill:");
            const SkillDef *skills = skills_for_class(g_player.pc);
            for (int i = 0; i < MAX_SKILLS; i++) {
                const SkillDef *sk = &skills[i];
                int slvl = g_player.skill_level[i];
                int cost = sk->mana_cost + (slvl - 1) * 2;
                bool can = (g_player.current_mana >= cost);
                char lbl[80];
                snprintf(lbl, sizeof(lbl), "%s (Lv%d) [%dMP]", sk->name, slvl, cost);
                Rectangle sb = { 30, (float)(by + i * 56), 600, 48 };
                if (ui_button(sb, lbl, can)) {
                    int prev_e = cs->enemy.current_life;
                    int prev_p = g_player.current_life;
                    combat_action_skill(i);
                    if (cs->enemy.current_life < prev_e) { anim_enemy_hit(false); snd_skill(); }
                    if (g_player.current_life > prev_p)  { anim_player_hit(true);  }
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
                int id = g_player.bag_ids[i];
                if (id < 0 || g_items[id].type != ITEM_CONSUMABLE) continue;
                char lbl[64];
                snprintf(lbl, sizeof(lbl), "%s x%d", g_items[id].name, g_player.bag_qty[i]);
                Rectangle ib = { 30, (float)(by + shown * 56), 400, 48 };
                if (ui_button(ib, lbl, true)) {
                    int prev = g_player.current_life;
                    combat_action_use_item(id);
                    if (g_player.current_life > prev) anim_player_hit(true);
                    s_show_items = false;
                }
                shown++;
            }
            if (shown == 0) txt(30, (float)by, 17, C_DIM, "No consumables.");
            Rectangle cancel2 = { 450, (float)by, 140, 48 };
            if (ui_button(cancel2, "Cancel", true)) s_show_items = false;
        }
    }

    /* Combat over overlay */
    if (cs->combat_over) {
        s_show_skills = false; s_show_items = false;
        Rectangle overlay = { 300, 280, 424, 220 };
        ui_panel(overlay, (Color){10,10,20,230}, cs->player_won ? C_XP : C_RED);

        if (cs->fled) {
            txt_c((Rectangle){300,300,424,40}, 22, C_GOLD, "You fled the battle!");
            Rectangle rb = { 380, 400, 260, 50 };
            if (ui_button(rb, "Return to Temple", true)) {
                g_scene_dirty = true;
                anim_fade_to(SCENE_HUB);
            }
        } else if (cs->player_won) {
            txt_c((Rectangle){300,300,424,40}, 28, C_XP, "Victory!");
            txt(340, 346, 17, C_GOLD,
                TextFormat("+%d XP   +%d Gold",
                    cs->enemy.xp_reward, cs->enemy.gold_reward));
            if (g_player.gw_complete[cs->gateway])
                txt(340, 372, 19, C_GREEN, "Gateway Complete!");

            Rectangle rb = { 380, 418, 260, 50 };
            if (ui_button(rb, "Continue", true)) {
                player_rest(&g_player);
                g_scene_dirty = true;
                if (g_player.stat_points > 0 || g_player.skill_points > 0) {
                    snd_level_up();
                    anim_fade_to(SCENE_LEVEL_UP);
                } else if (g_player.gw_complete[GW_DARK_RIFT]) {
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
                player_rest(&g_player);
                g_scene_dirty = true;
            }
            if (ui_button(give, "Give Up", true)) {
                snd_game_over();
                anim_fade_to(SCENE_GAME_OVER);
            }
        }
    }
}

/* ── INVENTORY ── */
static int s_inv_selected = -1;

void ui_draw_inventory(void)
{
    txt(20, 16, 26, C_GOLD, "Equipment & Inventory");

    Rectangle eq_panel = { 20, 60, 300, 180 };
    ui_panel(eq_panel, C_PANEL, C_BORDER);
    txt(34, 70, 17, C_WHITE, "Equipped");
    static const char *slot_labels[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_player.equip[s];
        const char *nm = (eid >= 0) ? g_items[eid].name : "(empty)";
        Rectangle sr = { 34, (float)(102 + s * 44), 272, 36 };
        if (ui_button(sr, TextFormat("%s: %s", slot_labels[s], nm), true)
            && eid >= 0)
        {
            player_unequip(&g_player, (EquipSlot)s);
            s_inv_selected = -1;
            g_scene_dirty = true;
        }
    }

    Rectangle bag_panel = { 20, 260, 300, SCREEN_H - 320 };
    ui_panel(bag_panel, C_PANEL, C_BORDER);
    txt(34, 270, 17, C_WHITE, "Bag");
    int row = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        int id = g_player.bag_ids[i];
        if (id < 0) continue;
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s%s", g_items[id].name,
            (g_items[id].type == ITEM_CONSUMABLE) ?
                TextFormat(" x%d", g_player.bag_qty[i]) : "");
        Rectangle br = { 34, (float)(296 + row * 44), 272, 36 };
        bool sel = (s_inv_selected == i);
        DrawRectangleRec(br, sel ? (Color){60,60,110,255} : C_BTN);
        DrawRectangleLinesEx(br, 1, C_BORDER);
        txt((int)br.x + 8, (int)br.y + 10, 15, C_WHITE, lbl);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, br) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            s_inv_selected = (sel ? -1 : i);
            snd_click();
        }
        if (++row >= 10) break;
    }

    Rectangle detail = { 340, 60, SCREEN_W - 360, SCREEN_H - 120 };
    ui_panel(detail, C_PANEL, C_BORDER);

    if (s_inv_selected >= 0 && g_player.bag_ids[s_inv_selected] >= 0) {
        int id = g_player.bag_ids[s_inv_selected];
        const ItemDef *it = &g_items[id];
        txt(360, 76, 22, C_GOLD, it->name);
        static const char *type_names[] = { "None","Weapon","Armor","Accessory","Consumable" };
        txt(360, 104, 15, C_DIM, type_names[it->type]);
        txt(360, 124, 15, C_GOLD, TextFormat("Sell value: %d gold", it->price/2));

        int dy = 154;
        if (it->bonus_life) { txt(360, dy, 15, C_HP,    TextFormat("+%d Life",    it->bonus_life)); dy+=22; }
        if (it->bonus_mana) { txt(360, dy, 15, C_MP,    TextFormat("+%d Mana",    it->bonus_mana)); dy+=22; }
        if (it->bonus_str)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Strength",it->bonus_str));  dy+=22; }
        if (it->bonus_spd)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Speed",   it->bonus_spd));  dy+=22; }
        if (it->bonus_def)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Defense", it->bonus_def));  dy+=22; }

        if (it->type == ITEM_WEAPON || it->type == ITEM_ARMOR || it->type == ITEM_ACCESSORY) {
            Rectangle eb = { 360, (float)(dy + 20), 150, 44 };
            if (ui_button(eb, "Equip", true)) {
                player_equip(&g_player, id);
                s_inv_selected = -1;
                g_scene_dirty = true;
            }
        }
        if (it->type == ITEM_CONSUMABLE) {
            Rectangle ub = { 360, (float)(dy + 20), 120, 44 };
            if (ui_button(ub, "Use", true)) {
                player_use_consumable(&g_player, id);
                s_inv_selected = -1;
            }
        }
        Rectangle sb = { 360, (float)(dy + 74), 120, 44 };
        if (ui_button(sb, "Sell", true)) {
            g_player.gold += it->price / 2;
            player_remove_item(&g_player, id);
            s_inv_selected = -1;
            g_scene_dirty = true;
        }
    } else {
        txt(360, 140, 17, C_DIM, "Select an item from your bag.");
    }

    Rectangle back = { 340, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_inv_selected = -1;
        anim_fade_to(SCENE_HUB);
    }
}

/* ── SHOP ── */
static int s_shop_sel = -1;

void ui_draw_shop(void)
{
    txt(20, 16, 26, C_GOLD, "Shop");
    txt(20, 50, 19, C_GOLD, TextFormat("Your Gold: %d", g_player.gold));

    Rectangle list_panel = { 20, 80, 380, SCREEN_H - 140 };
    ui_panel(list_panel, C_PANEL, C_BORDER);
    for (int i = 0; i < g_shop.count; i++) {
        int id = g_shop.item_ids[i];
        const ItemDef *it = &g_items[id];
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s  [%dg]", it->name, it->price);
        Rectangle ir = { 34, (float)(94 + i * 48), 352, 40 };
        bool sel = (s_shop_sel == i);
        DrawRectangleRec(ir, sel ? (Color){60,60,110,255} : C_BTN);
        DrawRectangleLinesEx(ir, 1, C_BORDER);
        Color tc = (g_player.gold >= it->price) ? C_WHITE : C_DIM;
        txt((int)ir.x + 8, (int)ir.y + 12, 15, tc, lbl);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, ir) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            s_shop_sel = (sel ? -1 : i);
            snd_click();
        }
    }

    Rectangle det = { 420, 80, SCREEN_W - 440, SCREEN_H - 140 };
    ui_panel(det, C_PANEL, C_BORDER);

    if (s_shop_sel >= 0 && s_shop_sel < g_shop.count) {
        int id = g_shop.item_ids[s_shop_sel];
        const ItemDef *it = &g_items[id];
        txt(436, 96, 22, C_GOLD, it->name);
        txt(436, 124, 17, C_GOLD, TextFormat("Price: %d gold", it->price));

        int dy = 158;
        static const char *type_names[] = {"None","Weapon","Armor","Accessory","Consumable"};
        txt(436, dy, 15, C_DIM, type_names[it->type]); dy += 28;
        if (it->bonus_life) { txt(436, dy, 15, C_HP,    TextFormat("+%d Life",    it->bonus_life)); dy+=22; }
        if (it->bonus_mana) { txt(436, dy, 15, C_MP,    TextFormat("+%d Mana",    it->bonus_mana)); dy+=22; }
        if (it->bonus_str)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Strength",it->bonus_str));  dy+=22; }
        if (it->bonus_spd)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Speed",   it->bonus_spd));  dy+=22; }
        if (it->bonus_def)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Defense", it->bonus_def));  dy+=22; }

        bool can = (g_player.gold >= it->price);
        Rectangle buy_btn = { 436, (float)(dy + 30), 140, 48 };
        if (ui_button(buy_btn, "Buy", can)) {
            if (player_add_item(&g_player, id)) {
                g_player.gold -= it->price;
                g_scene_dirty = true;
            }
        }
        if (!can) txt(436, (float)(dy + 84), 15, C_RED, "Not enough gold!");
    } else {
        txt(436, 160, 17, C_DIM, "Select an item.");
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_shop_sel = -1;
        anim_fade_to(SCENE_HUB);
    }
}

/* ── SKILLS ── */
void ui_draw_skills(void)
{
    txt(20, 16, 26, C_GOLD, "Skill Master");
    txt(20, 50, 19, C_WHITE, TextFormat("Skill Points: %d", g_player.skill_points));
    txt(300, 50, 18, C_DIM, TextFormat("Class: %s", class_name(g_player.pc)));

    const SkillDef *skills = skills_for_class(g_player.pc);
    for (int i = 0; i < MAX_SKILLS; i++) {
        const SkillDef *sk = &skills[i];
        int slvl = g_player.skill_level[i];
        int cost = sk->mana_cost + (slvl - 1) * 2;
        Rectangle row = { 20, (float)(90 + i * 110), SCREEN_W - 40, 100 };
        ui_panel(row, C_PANEL, C_BORDER);

        txt(36, (int)row.y + 12, 20, C_GOLD, sk->name);
        txt(250,(int)row.y + 14, 15,
            slvl >= sk->max_level ? C_GREEN : C_WHITE,
            TextFormat("Lv %d / %d", slvl, sk->max_level));
        txt(360,(int)row.y + 14, 15, C_MP, TextFormat("MP cost: %d", cost));
        txt(36, (int)row.y + 42, 14, C_DIM, sk->desc);

        static const char *eff[] = {
            "Damage","Heal","Buff","Debuff","DoT","Stun","Dodge","Multi-hit"
        };
        txt(36, (int)row.y + 64, 13, C_DIM,
            TextFormat("Effect: %s  Base: %d", eff[sk->effect], sk->base_value));

        bool can = (g_player.skill_points > 0 && slvl < sk->max_level);
        Rectangle up = { (float)(SCREEN_W - 170), row.y + 28, 140, 44 };
        if (ui_button(up, "Upgrade", can)) {
            g_player.skill_level[i]++;
            g_player.skill_points--;
            g_scene_dirty = true;
        }
    }
    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) anim_fade_to(SCENE_HUB);
}

/* ── LEVEL UP ── */
void ui_draw_level_up(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("LEVEL UP!", 46)/2, 60, 46, C_GOLD, "LEVEL UP!");
    txt(cx - txt_w(TextFormat("You are now Level %d!", g_player.level), 22)/2,
        118, 22, C_WHITE, TextFormat("You are now Level %d!", g_player.level));
    txt(cx - 160, 160, 19, C_WHITE, TextFormat("Stat points:  %d", g_player.stat_points));
    txt(cx - 160, 184, 17, C_DIM,   TextFormat("Skill points: %d", g_player.skill_points));

    static const struct { const char *lbl; int field; int delta; } stats[] = {
        { "+Life (+10)",    0, 10 },
        { "+Mana (+10)",    1, 10 },
        { "+Strength (+2)", 2,  2 },
        { "+Speed (+2)",    3,  2 },
        { "+Defense (+1)",  4,  1 },
    };
    for (int i = 0; i < 5; i++) {
        Rectangle btn = { (float)(cx - 150), (float)(230 + i * 60), 300, 48 };
        bool can = (g_player.stat_points > 0);
        if (ui_button(btn, stats[i].lbl, can) && can) {
            g_player.stat_points--;
            int *fields[] = {
                &g_player.base.max_life, &g_player.base.max_mana,
                &g_player.base.strength, &g_player.base.speed,
                &g_player.base.defense
            };
            *fields[stats[i].field] += stats[i].delta;
            if (stats[i].field == 0) g_player.current_life += stats[i].delta;
            if (stats[i].field == 1) g_player.current_mana += stats[i].delta;
            g_scene_dirty = true;
        }
    }

    Rectangle done_btn = { (float)(cx - 100), 556, 200, 50 };
    bool done = (g_player.stat_points == 0);
    if (ui_button(done_btn, done ? "Done!" : "Skip for now", true)) {
        if (g_player.skill_points > 0) anim_fade_to(SCENE_SKILLS);
        else anim_fade_to(SCENE_HUB);
    }
}

/* ── GATEWAY SELECT ── */
void ui_draw_gateway_select(void)
{
    GatewayId gw = g_combat.gateway;
    static const char *gw_labels[] = { "Human Gateway", "Monster Portal", "Dark Rift" };
    static const char *gw_flavor[] = {
        "20 floors of human warriors - main story path.",
        "10 floors of monsters - optional challenge.",
        "5 floors of darkness - post-game ultimate challenge."
    };
    int cx = SCREEN_W / 2;
    txt(cx - txt_w(gw_labels[gw], 30)/2, 30, 30, C_GOLD, gw_labels[gw]);
    txt(cx - txt_w(gw_flavor[gw], 15)/2, 72, 15, C_DIM, gw_flavor[gw]);

    int prog = g_player.gw_progress[gw];
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
                g_scene = SCENE_COMBAT;
                g_scene_dirty = true;
            }
        } else {
            txt((int)row.x + (int)row.width - 90, (int)row.y + 26, 17, C_DIM, "LOCKED");
        }
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) anim_fade_to(SCENE_HUB);
}

/* ── GAME OVER ── */
void ui_draw_game_over(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("GAME OVER", 58)/2, 160, 58, C_RED, "GAME OVER");
    txt(cx - 200, 248, 21, C_WHITE,
        TextFormat("You fell as a Level %d %s.", g_player.level, class_name(g_player.pc)));
    txt(cx - 80, 282, 17, C_GOLD, TextFormat("Gold earned: %d", g_player.gold));

    Rectangle replay = { (float)(cx - 110), 358, 220, 54 };
    if (ui_button(replay, "Play Again", true)) {
        g_scene_dirty = true;
        s_naming = false;
        s_name_len = 4;
        memcpy(s_name_buf, "Hero", 5);
        anim_fade_to(SCENE_TITLE);
    }
}

/* ── VICTORY ── */
void ui_draw_victory(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("VICTORY!", 58)/2, 100, 58, C_GOLD, "VICTORY!");
    txt(cx - txt_w("You have conquered the Dark Rift!", 22)/2,
        174, 22, C_WHITE, "You have conquered the Dark Rift!");
    txt(cx - txt_w("Warlord Baka has been defeated.", 15)/2,
        210, 15, C_DIM, "Warlord Baka has been defeated.");
    txt(cx - 100, 268, 19, C_WHITE, TextFormat("Final Level: %d", g_player.level));
    txt(cx - 100, 294, 19, C_GOLD,  TextFormat("Gold: %d",        g_player.gold));

    Rectangle hub_btn  = { (float)(cx - 230), 378, 200, 54 };
    Rectangle play_btn = { (float)(cx + 30),  378, 200, 54 };
    if (ui_button(hub_btn, "Return to Hub", true)) anim_fade_to(SCENE_HUB);
    if (ui_button(play_btn,"Play Again",    true)) {
        g_scene_dirty = true;
        s_naming = false; s_name_len = 4; memcpy(s_name_buf, "Hero", 5);
        anim_fade_to(SCENE_TITLE);
    }
}
