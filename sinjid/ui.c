#include "ui.h"
#include "game.h"
#include "combat.h"
#include "player.h"
#include "hub.h"
#include "data.h"
#include <string.h>
#include <stdio.h>

/* ── palette ── */

static const Color C_PANEL    = {  28,  28,  45, 255 };
static const Color C_BORDER   = {  70,  70, 120, 255 };
static const Color C_GOLD     = { 220, 180,  40, 255 };
static const Color C_WHITE    = { 230, 230, 230, 255 };
static const Color C_DIM      = { 100, 100, 130, 255 };
static const Color C_BTN      = {  45,  45,  80, 255 };
static const Color C_BTN_HOV  = {  70,  70, 130, 255 };
static const Color C_BTN_DIS  = {  30,  30,  45, 255 };
static const Color C_HP       = { 200,  40,  40, 255 };
static const Color C_MP       = {  40,  80, 200, 255 };
static const Color C_XP       = {  40, 180,  80, 255 };
static const Color C_GREEN    = {  60, 200,  60, 255 };
static const Color C_RED      = { 220,  60,  60, 255 };

/* ── helpers ── */
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
        DrawRectangle(x, y, filled, h, fill);
    }
    DrawRectangleLines(x, y, w, h, C_BORDER);
}

void ui_text_center(Rectangle r, const char *text, int font_size, Color col)
{
    int tw = MeasureText(text, font_size);
    int tx = (int)(r.x + (r.width  - tw) / 2);
    int ty = (int)(r.y + (r.height - font_size) / 2);
    DrawText(text, tx, ty, font_size, col);
}

bool ui_button(Rectangle r, const char *label, bool enabled)
{
    Vector2 mouse = GetMousePosition();
    bool hovered  = enabled && CheckCollisionPointRec(mouse, r);
    Color bg      = !enabled ? C_BTN_DIS : (hovered ? C_BTN_HOV : C_BTN);
    Color tc      = !enabled ? C_DIM     : C_WHITE;
    ui_panel(r, bg, C_BORDER);
    ui_text_center(r, label, 18, tc);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

/* ── name-input state (class select) ── */
static char s_name_buf[32] = "Hero";
static int  s_name_len     = 4;
static bool s_naming       = false;
static PlayerClass s_chosen_class = CLASS_WARRIOR;

/* ── TITLE ── */
void ui_draw_title(void)
{
    int cx = SCREEN_W / 2;
    DrawText("SINJID",
        cx - MeasureText("SINJID", 72) / 2, 160, 72, C_GOLD);
    DrawText("Shadow of the Warrior",
        cx - MeasureText("Shadow of the Warrior", 28) / 2, 248, 28, C_WHITE);
    DrawText("A fan-made simplified clone",
        cx - MeasureText("A fan-made simplified clone", 18) / 2, 290, 18, C_DIM);

    Rectangle btn = { cx - 120, 400, 240, 50 };
    if (ui_button(btn, "Begin Your Journey", true))
        g_scene = SCENE_CLASS_SELECT;

    DrawText("ESC to quit | F1 debug overlay",
        cx - MeasureText("ESC to quit | F1 debug overlay", 16) / 2,
        SCREEN_H - 40, 16, C_DIM);
}

/* ── CLASS SELECT ── */
void ui_draw_class_select(void)
{
    int cx = SCREEN_W / 2;
    DrawText("Choose Your Class",
        cx - MeasureText("Choose Your Class", 32) / 2, 40, 32, C_GOLD);

    static const struct { PlayerClass pc; const char *name; const char *desc; Color col; } classes[] = {
        { CLASS_WARRIOR, "Warrior", "High strength & defense.\nPowerful melee skills.", (Color){180,80,80,255} },
        { CLASS_ROGUE,   "Rogue",   "High speed & agility.\nPoison and multi-hit skills.", (Color){80,180,80,255} },
        { CLASS_MAGE,    "Mage",    "High mana & magic.\nDevastating spell skills.", (Color){80,80,200,255} },
    };

    if (!s_naming) {
        for (int i = 0; i < 3; i++) {
            Rectangle box = { 80 + i * 300, 120, 260, 200 };
            bool sel = (s_chosen_class == classes[i].pc);
            ui_panel(box, sel ? (Color){40,40,80,255} : C_PANEL,
                     sel ? C_GOLD : C_BORDER);
            /* class colour swatch */
            DrawRectangle((int)box.x + 10, (int)box.y + 10, 40, 40, classes[i].col);
            DrawText(classes[i].name, (int)box.x + 60, (int)box.y + 18, 24, C_WHITE);

            Stats st = class_base_stats(classes[i].pc);
            DrawText(TextFormat("Life:  %d", st.max_life),  (int)box.x+14, (int)box.y+70,  16, C_HP);
            DrawText(TextFormat("Mana:  %d", st.max_mana),  (int)box.x+14, (int)box.y+90,  16, C_MP);
            DrawText(TextFormat("STR:   %d", st.strength),  (int)box.x+14, (int)box.y+110, 16, C_WHITE);
            DrawText(TextFormat("SPD:   %d", st.speed),     (int)box.x+14, (int)box.y+130, 16, C_WHITE);
            DrawText(TextFormat("DEF:   %d", st.defense),   (int)box.x+14, (int)box.y+150, 16, C_WHITE);

            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                s_chosen_class = classes[i].pc;
        }

        DrawText("Warrior: tanky melee fighter with buffs and stuns",       80, 340, 16, C_DIM);
        DrawText("Rogue:   fast striker with poison, dodge, and multi-hit", 80, 360, 16, C_DIM);
        DrawText("Mage:    fragile caster with high-damage spells and heal",80, 380, 16, C_DIM);

        Rectangle next = { cx - 100, 440, 200, 48 };
        if (ui_button(next, "Choose Name ->", true))
            s_naming = true;

        Rectangle back = { cx - 100, 500, 200, 40 };
        if (ui_button(back, "<- Back", true))
            g_scene = SCENE_TITLE;
    } else {
        /* Name entry */
        DrawText("Enter your name:",
            cx - MeasureText("Enter your name:", 24) / 2, 200, 24, C_WHITE);

        Rectangle nbox = { cx - 160, 250, 320, 48 };
        ui_panel(nbox, C_PANEL, C_GOLD);
        DrawText(s_name_buf, (int)nbox.x + 12, (int)nbox.y + 14, 24, C_WHITE);
        /* blinking cursor */
        if (((int)(GetTime() * 2)) % 2 == 0)
            DrawText("_", (int)nbox.x + 12 + MeasureText(s_name_buf, 24),
                     (int)nbox.y + 14, 24, C_GOLD);

        /* character input */
        int ch;
        while ((ch = GetCharPressed()) != 0) {
            if (ch >= 32 && s_name_len < 30) {
                s_name_buf[s_name_len++] = (char)ch;
                s_name_buf[s_name_len]   = '\0';
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE) && s_name_len > 0) {
            s_name_buf[--s_name_len] = '\0';
        }

        Rectangle start = { cx - 100, 340, 200, 48 };
        bool name_ok = s_name_len > 0;
        if (ui_button(start, "Start Game!", name_ok) && name_ok) {
            player_init(&g_player, s_name_buf, s_chosen_class);
            g_scene  = SCENE_HUB;
            s_naming = false;
        }
        Rectangle back2 = { cx - 100, 400, 200, 40 };
        if (ui_button(back2, "<- Back", true))
            s_naming = false;
    }
}

/* ── HUB ── */
void ui_draw_hub(void)
{
    /* Left panel: locations */
    Rectangle left = { 20, 20, 420, SCREEN_H - 40 };
    ui_panel(left, C_PANEL, C_BORDER);
    DrawText("Shadow Temple", 36, 34, 24, C_GOLD);
    DrawText("_________________________________", 36, 60, 16, C_BORDER);

    static const HubLocation locs[] = {
        HUB_GW_HUMAN, HUB_GW_MONSTER, HUB_GW_DARK,
        HUB_TRAINING,
        HUB_SHOP_BASIC, HUB_SHOP_MID, HUB_SHOP_ADV,
        HUB_SKILLS, HUB_INVENTORY
    };
    static const int LOC_COUNT = 9;

    for (int i = 0; i < LOC_COUNT; i++) {
        HubLocation loc = locs[i];
        bool unlocked   = hub_location_unlocked(loc);
        Rectangle btn   = { 36, 80 + i * 60, 388, 48 };
        const char *lbl = hub_location_name(loc);
        char buf[64];
        if (!unlocked) snprintf(buf, sizeof(buf), "[LOCKED] %s", lbl);
        else           snprintf(buf, sizeof(buf), "%s", lbl);
        if (ui_button(btn, buf, unlocked))
            hub_enter(loc);
    }

    /* Right panel: player stats */
    Rectangle right = { 460, 20, SCREEN_W - 480, SCREEN_H - 40 };
    ui_panel(right, C_PANEL, C_BORDER);

    int rx = 474, ry = 34;
    DrawText(TextFormat("%s the %s", g_player.name, class_name(g_player.pc)),
        rx, ry, 22, C_GOLD);
    DrawText(TextFormat("Level %d", g_player.level), rx, ry + 30, 18, C_WHITE);
    DrawText(TextFormat("Gold: %d", g_player.gold),  rx, ry + 52, 18, C_GOLD);

    /* XP bar */
    DrawText("XP:", rx, ry + 80, 16, C_DIM);
    ui_bar(rx + 30, ry + 80, 200, 14,
           g_player.xp, g_player.xp_to_next, C_XP, C_BTN);
    DrawText(TextFormat("%d / %d", g_player.xp, g_player.xp_to_next),
        rx + 235, ry + 80, 14, C_DIM);

    /* HP/MP bars */
    int max_life = player_effective_life();
    int max_mana = player_effective_mana();
    DrawText("HP:", rx, ry + 106, 16, C_DIM);
    ui_bar(rx + 30, ry + 106, 200, 14,
           g_player.current_life, max_life, C_HP, C_BTN);
    DrawText(TextFormat("%d/%d", g_player.current_life, max_life),
        rx + 235, ry + 106, 14, C_DIM);

    DrawText("MP:", rx, ry + 128, 16, C_DIM);
    ui_bar(rx + 30, ry + 128, 200, 14,
           g_player.current_mana, max_mana, C_MP, C_BTN);
    DrawText(TextFormat("%d/%d", g_player.current_mana, max_mana),
        rx + 235, ry + 128, 14, C_DIM);

    /* Stats */
    DrawText("── Stats ──────────────", rx, ry + 160, 16, C_BORDER);
    DrawText(TextFormat("Strength:  %d", player_effective_str()), rx, ry+182, 16, C_WHITE);
    DrawText(TextFormat("Speed:     %d", player_effective_spd()), rx, ry+202, 16, C_WHITE);
    DrawText(TextFormat("Defense:   %d", player_effective_def()), rx, ry+222, 16, C_WHITE);

    /* Equipment */
    DrawText("── Equipment ──────────", rx, ry + 256, 16, C_BORDER);
    static const char *slot_names[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_player.equip[s];
        const char *iname = (eid >= 0) ? g_items[eid].name : "(none)";
        DrawText(TextFormat("%s: %s", slot_names[s], iname),
            rx, ry + 278 + s * 22, 16, (eid >= 0) ? C_WHITE : C_DIM);
    }

    /* Gateway progress */
    DrawText("── Gateways ───────────", rx, ry + 360, 16, C_BORDER);
    static const char *gw_names[] = { "Human Gateway", "Monster Portal", "Dark Rift" };
    for (int g = 0; g < NUM_GATEWAYS; g++) {
        int  prog = g_player.gw_progress[g];
        bool comp = g_player.gw_complete[g];
        Color gc  = comp ? C_GREEN : (prog > 0 ? C_GOLD : C_DIM);
        DrawText(TextFormat("%s: %s%d/%d",
            gw_names[g], comp ? "COMPLETE " : "", prog, GATEWAY_DEPTH),
            rx, ry + 382 + g * 22, 15, gc);
    }

    /* Pending level-up hint */
    if (g_player.stat_points > 0 || g_player.skill_points > 0) {
        DrawText(TextFormat("! Level-up points available (%d stat, %d skill)",
            g_player.stat_points, g_player.skill_points),
            rx, ry + 455, 16, C_GOLD);
        Rectangle lub = { (float)rx, (float)(ry + 478), 220, 38 };
        if (ui_button(lub, "Allocate Points", true))
            g_scene = SCENE_LEVEL_UP;
    }
}

/* ── COMBAT ── */
static bool s_show_skills = false;
static bool s_show_items  = false;

/* draw a combatant card */
static void draw_combatant(int x, int y, int w, const char *name,
    int cur_life, int max_life, int cur_mana, int max_mana,
    Color body_col, StatusFlags status)
{
    /* body rectangle */
    DrawRectangle(x, y, w, 120, body_col);
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,120}, 2, C_BORDER);
    /* name */
    DrawText(name, x + 8, y + 8, 20, C_WHITE);
    /* HP bar */
    DrawText("HP", x + 8, y + 38, 14, C_DIM);
    ui_bar(x + 34, y + 38, w - 44, 14, cur_life, max_life, C_HP, C_BTN);
    DrawText(TextFormat("%d/%d", cur_life, max_life), x + 8, y + 56, 13, C_DIM);
    /* MP bar */
    DrawText("MP", x + 8, y + 74, 14, C_DIM);
    ui_bar(x + 34, y + 74, w - 44, 14, cur_mana, max_mana, C_MP, C_BTN);
    DrawText(TextFormat("%d/%d", cur_mana, max_mana), x + 8, y + 92, 13, C_DIM);
    /* status icons */
    int sx = x + 8, sy = y + 108;
    if (status & STATUS_POISONED)  { DrawText("PSN", sx, sy, 12, C_GREEN);    sx += 30; }
    if (status & STATUS_STUNNED)   { DrawText("STN", sx, sy, 12, C_GOLD);     sx += 30; }
    if (status & STATUS_SLOWED)    { DrawText("SLW", sx, sy, 12, C_MP);       sx += 30; }
    if (status & STATUS_POWERED)   { DrawText("PWR", sx, sy, 12, C_RED);      sx += 30; }
    if (status & STATUS_SHIELDED)  { DrawText("SHD", sx, sy, 12, C_WHITE);    sx += 30; }
    if (status & STATUS_DODGING)   { DrawText("DDG", sx, sy, 12, C_XP);       sx += 30; }
    (void)sx;
}

void ui_draw_combat(void)
{
    CombatState *cs = &g_combat;

    /* Auto-advance enemy turn */
    if (!cs->player_turn && !cs->combat_over) {
        combat_enemy_turn();
    }

    /* ── combatant cards ── */
    draw_combatant(30, 30, 300, g_player.name,
        g_player.current_life, player_effective_life(),
        g_player.current_mana, player_effective_mana(),
        (Color){40,60,120,255}, g_player.status);

    draw_combatant(SCREEN_W - 330, 30, 300, cs->enemy.name,
        cs->enemy.current_life, cs->enemy.base_stats.max_life,
        cs->enemy.current_mana, cs->enemy.base_stats.max_mana,
        cs->enemy.color, cs->enemy.status);

    /* turn indicator */
    const char *turn_txt = cs->combat_over ? "" :
        (cs->player_turn ? ">> Your turn <<" : "Enemy thinking...");
    DrawText(turn_txt,
        SCREEN_W/2 - MeasureText(turn_txt, 20)/2, 60, 20, C_GOLD);

    /* ── combat log ── */
    Rectangle log_box = { 340, 20, SCREEN_W - 680, 380 };
    ui_panel(log_box, C_PANEL, C_BORDER);
    DrawText("Combat Log", (int)log_box.x + 8, (int)log_box.y + 6, 14, C_DIM);
    int visible = cs->log_count;
    if (visible > 7) visible = 7;
    for (int i = 0; i < visible; i++) {
        int li = cs->log_count - visible + i;
        DrawText(cs->log[li],
            (int)log_box.x + 8,
            (int)log_box.y + 26 + i * 48,
            16, (i == visible - 1) ? C_WHITE : C_DIM);
    }

    /* ── action bar (only when player's turn) ── */
    if (cs->player_turn && !cs->combat_over) {
        int by = 430;

        if (!s_show_skills && !s_show_items) {
            Rectangle ra = {  30, (float)by, 180, 48 };
            Rectangle rs = { 220, (float)by, 180, 48 };
            Rectangle ri = { 410, (float)by, 180, 48 };
            Rectangle rf = { 600, (float)by, 180, 48 };

            if (ui_button(ra, "Attack",  true))  { s_show_skills = false; s_show_items = false; combat_action_attack(); }
            if (ui_button(rs, "Skills",  true))  { s_show_skills = true;  s_show_items = false; }
            if (ui_button(ri, "Items",   true))  { s_show_items  = true;  s_show_skills = false; }
            if (ui_button(rf, "Flee",    true))  { combat_action_flee(); }
        }

        /* skill submenu */
        if (s_show_skills) {
            DrawText("Choose a skill:", 30, (float)by - 24, 18, C_GOLD);
            const SkillDef *skills = skills_for_class(g_player.pc);
            for (int i = 0; i < MAX_SKILLS; i++) {
                const SkillDef *sk = &skills[i];
                int slvl = g_player.skill_level[i];
                int cost = sk->mana_cost + (slvl - 1) * 2;
                bool can = (g_player.current_mana >= cost);
                char lbl[64];
                snprintf(lbl, sizeof(lbl), "%s (Lv%d) [%dMP]", sk->name, slvl, cost);
                Rectangle sb = { 30, (float)(by + i * 56), 600, 48 };
                if (ui_button(sb, lbl, can)) {
                    combat_action_skill(i);
                    s_show_skills = false;
                }
            }
            Rectangle cancel = { 650, (float)by, 140, 48 };
            if (ui_button(cancel, "Cancel", true)) s_show_skills = false;
        }

        /* items submenu */
        if (s_show_items) {
            DrawText("Use an item:", 30, (float)by - 24, 18, C_GOLD);
            int shown = 0;
            for (int i = 0; i < MAX_INVENTORY && shown < 6; i++) {
                int id = g_player.bag_ids[i];
                if (id < 0 || g_items[id].type != ITEM_CONSUMABLE) continue;
                char lbl[64];
                snprintf(lbl, sizeof(lbl), "%s x%d", g_items[id].name, g_player.bag_qty[i]);
                Rectangle ib = { 30, (float)(by + shown * 56), 400, 48 };
                if (ui_button(ib, lbl, true)) {
                    combat_action_use_item(id);
                    s_show_items = false;
                }
                shown++;
            }
            if (shown == 0)
                DrawText("No consumables in bag.", 30, (float)by, 18, C_DIM);
            Rectangle cancel2 = { 450, (float)by, 140, 48 };
            if (ui_button(cancel2, "Cancel", true)) s_show_items = false;
        }
    }

    /* ── combat over overlay ── */
    if (cs->combat_over) {
        s_show_skills = false;
        s_show_items  = false;

        Rectangle overlay = { 300, 280, 424, 220 };
        ui_panel(overlay, (Color){10,10,20,230}, cs->player_won ? C_XP : C_RED);

        if (cs->fled) {
            ui_text_center((Rectangle){300,300,424,40}, "You fled the battle!", 24, C_GOLD);
            Rectangle rb = { 380, 400, 260, 50 };
            if (ui_button(rb, "Return to Temple", true)) {
                g_scene = SCENE_HUB;
            }
        } else if (cs->player_won) {
            ui_text_center((Rectangle){300,300,424,40}, "Victory!", 30, C_XP);
            DrawText(TextFormat("+%d XP   +%d Gold",
                cs->enemy.xp_reward, cs->enemy.gold_reward),
                340, 348, 18, C_GOLD);
            if (g_player.gw_complete[cs->gateway])
                DrawText("Gateway Complete!", 340, 374, 20, C_GREEN);

            Rectangle rb = { 380, 420, 260, 50 };
            if (ui_button(rb, "Continue", true)) {
                player_rest(&g_player);
                if (g_player.stat_points > 0 || g_player.skill_points > 0)
                    g_scene = SCENE_LEVEL_UP;
                else if (g_player.gw_complete[GW_DARK_RIFT])
                    g_scene = SCENE_VICTORY;
                else
                    g_scene = SCENE_HUB;
            }
        } else {
            ui_text_center((Rectangle){300,300,424,40}, "Defeated...", 30, C_RED);
            Rectangle retry = { 320, 400, 180, 50 };
            Rectangle give  = { 520, 400, 180, 50 };
            if (ui_button(retry, "Retry", true)) {
                combat_start(cs->gateway, cs->floor);
                player_rest(&g_player);
            }
            if (ui_button(give, "Give Up", true)) {
                g_scene = SCENE_GAME_OVER;
            }
        }
    }
}

/* ── INVENTORY ── */
static int s_inv_selected = -1;

void ui_draw_inventory(void)
{
    DrawText("Equipment & Inventory", 20, 16, 26, C_GOLD);

    /* Equipment slots */
    Rectangle eq_panel = { 20, 60, 300, 180 };
    ui_panel(eq_panel, C_PANEL, C_BORDER);
    DrawText("Equipped", 34, 70, 18, C_WHITE);
    static const char *slot_labels[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_player.equip[s];
        const char *nm = (eid >= 0) ? g_items[eid].name : "(empty)";
        Rectangle sr = { 34, (float)(102 + s * 44), 272, 36 };
        bool clicked = ui_button(sr, TextFormat("%s: %s", slot_labels[s], nm), true);
        if (clicked && eid >= 0) {
            player_unequip(&g_player, (EquipSlot)s);
            s_inv_selected = -1;
        }
    }

    /* Bag */
    Rectangle bag_panel = { 20, 260, 300, SCREEN_H - 320 };
    ui_panel(bag_panel, C_PANEL, C_BORDER);
    DrawText("Bag", 34, 270, 18, C_WHITE);
    int row = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        int id = g_player.bag_ids[i];
        if (id < 0) continue;
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s%s",
            g_items[id].name,
            (g_items[id].type == ITEM_CONSUMABLE) ?
                TextFormat(" x%d", g_player.bag_qty[i]) : "");
        Rectangle br = { 34, (float)(296 + row * 44), 272, 36 };
        bool sel = (s_inv_selected == i);
        Color bg = sel ? (Color){60,60,110,255} : C_BTN;
        DrawRectangleRec(br, bg);
        DrawRectangleLinesEx(br, 1, C_BORDER);
        DrawText(lbl, (int)br.x + 8, (int)br.y + 10, 16, C_WHITE);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, br) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            s_inv_selected = (sel ? -1 : i);
        row++;
        if (row >= 10) break;
    }

    /* Detail / action panel */
    Rectangle detail = { 340, 60, SCREEN_W - 360, SCREEN_H - 120 };
    ui_panel(detail, C_PANEL, C_BORDER);

    if (s_inv_selected >= 0 && g_player.bag_ids[s_inv_selected] >= 0) {
        int id = g_player.bag_ids[s_inv_selected];
        const ItemDef *it = &g_items[id];
        DrawText(it->name, 360, 76, 22, C_GOLD);
        static const char *type_names[] = { "None","Weapon","Armor","Accessory","Consumable" };
        DrawText(type_names[it->type], 360, 104, 16, C_DIM);
        DrawText(TextFormat("Sell value: %d gold", it->price/2), 360, 128, 16, C_GOLD);

        int dy = 158;
        if (it->bonus_life) { DrawText(TextFormat("+%d Life",    it->bonus_life), 360, dy, 16, C_HP);    dy+=22; }
        if (it->bonus_mana) { DrawText(TextFormat("+%d Mana",    it->bonus_mana), 360, dy, 16, C_MP);    dy+=22; }
        if (it->bonus_str)  { DrawText(TextFormat("+%d Strength",it->bonus_str),  360, dy, 16, C_WHITE); dy+=22; }
        if (it->bonus_spd)  { DrawText(TextFormat("+%d Speed",   it->bonus_spd),  360, dy, 16, C_WHITE); dy+=22; }
        if (it->bonus_def)  { DrawText(TextFormat("+%d Defense", it->bonus_def),  360, dy, 16, C_WHITE); dy+=22; }

        if (it->type == ITEM_WEAPON || it->type == ITEM_ARMOR || it->type == ITEM_ACCESSORY) {
            Rectangle equip_btn = { 360, (float)(dy + 20), 160, 44 };
            if (ui_button(equip_btn, "Equip", true)) {
                player_equip(&g_player, id);
                s_inv_selected = -1;
            }
        }
        if (it->type == ITEM_CONSUMABLE) {
            Rectangle use_btn = { 360, (float)(dy + 20), 120, 44 };
            if (ui_button(use_btn, "Use", true)) {
                player_use_consumable(&g_player, id);
                s_inv_selected = -1;
            }
        }
        Rectangle sell_btn = { 360, (float)(dy + 74), 120, 44 };
        if (ui_button(sell_btn, "Sell", true)) {
            g_player.gold += it->price / 2;
            player_remove_item(&g_player, id);
            s_inv_selected = -1;
        }
    } else {
        DrawText("Select an item from your bag.", 360, 140, 18, C_DIM);
    }

    Rectangle back = { 340, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_inv_selected = -1;
        g_scene = SCENE_HUB;
    }
}

/* ── SHOP ── */
static int s_shop_sel = -1;

void ui_draw_shop(void)
{
    DrawText("Shop", 20, 16, 28, C_GOLD);
    DrawText(TextFormat("Your Gold: %d", g_player.gold), 20, 52, 20, C_GOLD);

    /* Item list */
    Rectangle list_panel = { 20, 80, 380, SCREEN_H - 140 };
    ui_panel(list_panel, C_PANEL, C_BORDER);
    for (int i = 0; i < g_shop.count; i++) {
        int id = g_shop.item_ids[i];
        const ItemDef *it = &g_items[id];
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s  [%dg]", it->name, it->price);
        Rectangle ir = { 34, (float)(94 + i * 48), 352, 40 };
        bool sel = (s_shop_sel == i);
        Color bg  = sel ? (Color){60,60,110,255} : C_BTN;
        DrawRectangleRec(ir, bg);
        DrawRectangleLinesEx(ir, 1, C_BORDER);
        Color tc = (g_player.gold >= it->price) ? C_WHITE : C_DIM;
        DrawText(lbl, (int)ir.x + 8, (int)ir.y + 12, 16, tc);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, ir) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            s_shop_sel = (sel ? -1 : i);
    }

    /* Detail + buy panel */
    Rectangle det = { 420, 80, SCREEN_W - 440, SCREEN_H - 140 };
    ui_panel(det, C_PANEL, C_BORDER);

    if (s_shop_sel >= 0 && s_shop_sel < g_shop.count) {
        int id = g_shop.item_ids[s_shop_sel];
        const ItemDef *it = &g_items[id];
        DrawText(it->name, 436, 96, 22, C_GOLD);
        DrawText(TextFormat("Price: %d gold", it->price), 436, 126, 18, C_GOLD);

        int dy = 160;
        static const char *type_names[] = { "None","Weapon","Armor","Accessory","Consumable" };
        DrawText(type_names[it->type], 436, dy, 16, C_DIM); dy += 28;
        if (it->bonus_life) { DrawText(TextFormat("+%d Life",    it->bonus_life), 436, dy, 16, C_HP);    dy+=22; }
        if (it->bonus_mana) { DrawText(TextFormat("+%d Mana",    it->bonus_mana), 436, dy, 16, C_MP);    dy+=22; }
        if (it->bonus_str)  { DrawText(TextFormat("+%d Strength",it->bonus_str),  436, dy, 16, C_WHITE); dy+=22; }
        if (it->bonus_spd)  { DrawText(TextFormat("+%d Speed",   it->bonus_spd),  436, dy, 16, C_WHITE); dy+=22; }
        if (it->bonus_def)  { DrawText(TextFormat("+%d Defense", it->bonus_def),  436, dy, 16, C_WHITE); dy+=22; }

        bool can_afford = (g_player.gold >= it->price);
        Rectangle buy_btn = { 436, (float)(dy + 30), 140, 48 };
        if (ui_button(buy_btn, "Buy", can_afford)) {
            if (can_afford && player_add_item(&g_player, id)) {
                g_player.gold -= it->price;
            }
        }
        if (!can_afford)
            DrawText("Not enough gold!", 436, (float)(dy + 84), 16, C_RED);
    } else {
        DrawText("Select an item to view details.", 436, 160, 18, C_DIM);
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_shop_sel = -1;
        g_scene = SCENE_HUB;
    }
}

/* ── SKILLS ── */
void ui_draw_skills(void)
{
    DrawText("Skill Master", 20, 16, 28, C_GOLD);
    DrawText(TextFormat("Skill Points: %d", g_player.skill_points),
        20, 52, 20, C_WHITE);
    DrawText(TextFormat("Class: %s", class_name(g_player.pc)), 300, 52, 20, C_DIM);

    const SkillDef *skills = skills_for_class(g_player.pc);
    for (int i = 0; i < MAX_SKILLS; i++) {
        const SkillDef *sk = &skills[i];
        int slvl = g_player.skill_level[i];
        int cost  = sk->mana_cost + (slvl - 1) * 2;
        Rectangle row = { 20, (float)(90 + i * 110), SCREEN_W - 40, 100 };
        ui_panel(row, C_PANEL, C_BORDER);

        DrawText(sk->name, 36, (int)row.y + 12, 20, C_GOLD);
        DrawText(TextFormat("Lv %d / %d", slvl, sk->max_level),
            250, (int)row.y + 14, 16, slvl >= sk->max_level ? C_GREEN : C_WHITE);
        DrawText(TextFormat("Mana cost: %d", cost), 360, (int)row.y + 14, 16, C_MP);
        DrawText(sk->desc, 36, (int)row.y + 42, 15, C_DIM);
        /* effect summary */
        static const char *eff_names[] = {
            "Damage","Heal","Buff","Debuff","DoT","Stun","Dodge","Multi-hit"
        };
        DrawText(TextFormat("Effect: %s  Base: %d",
            eff_names[sk->effect], sk->base_value),
            36, (int)row.y + 64, 14, C_DIM);

        bool can_upgrade = (g_player.skill_points > 0 && slvl < sk->max_level);
        Rectangle up_btn = { (float)(SCREEN_W - 170), row.y + 26, 140, 44 };
        if (ui_button(up_btn, "Upgrade", can_upgrade)) {
            g_player.skill_level[i]++;
            g_player.skill_points--;
        }
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) g_scene = SCENE_HUB;
}

/* ── LEVEL UP ── */
void ui_draw_level_up(void)
{
    int cx = SCREEN_W / 2;
    DrawText("LEVEL UP!", cx - MeasureText("LEVEL UP!", 48) / 2, 60, 48, C_GOLD);
    DrawText(TextFormat("You are now Level %d!", g_player.level),
        cx - MeasureText(TextFormat("You are now Level %d!", g_player.level), 24)/2,
        120, 24, C_WHITE);

    DrawText(TextFormat("Stat points remaining: %d", g_player.stat_points),
        cx - 160, 168, 20, C_WHITE);
    DrawText(TextFormat("Skill points remaining: %d", g_player.skill_points),
        cx - 160, 194, 18, C_DIM);

    /* Stat allocation buttons */
    static const struct { const char *lbl; int field; int delta; Color col; } stats[] = {
        { "+Life (+10)",     0, 10, (Color){200,60,60,255}  },
        { "+Mana (+10)",     1, 10, (Color){60,80,200,255}  },
        { "+Strength (+2)",  2,  2, (Color){200,200,60,255} },
        { "+Speed (+2)",     3,  2, (Color){60,200,120,255} },
        { "+Defense (+1)",   4,  1, (Color){180,180,180,255}},
    };
    for (int i = 0; i < 5; i++) {
        Rectangle btn = { (float)(cx - 150), (float)(240 + i * 60), 300, 48 };
        bool can = (g_player.stat_points > 0);
        if (ui_button(btn, stats[i].lbl, can) && can) {
            g_player.stat_points--;
            int *fields[] = {
                &g_player.base.max_life,
                &g_player.base.max_mana,
                &g_player.base.strength,
                &g_player.base.speed,
                &g_player.base.defense
            };
            *fields[stats[i].field] += stats[i].delta;
            /* top up current life/mana when max increases */
            if (stats[i].field == 0) g_player.current_life += stats[i].delta;
            if (stats[i].field == 1) g_player.current_mana += stats[i].delta;
        }
    }

    /* Done button – only when no pending points or player chooses to defer */
    bool done = (g_player.stat_points == 0);
    Rectangle done_btn = { (float)(cx - 100), 560, 200, 50 };
    if (ui_button(done_btn, done ? "Done!" : "Skip for now", true)) {
        if (g_player.skill_points > 0)
            g_scene = SCENE_SKILLS;
        else
            g_scene = SCENE_HUB;
    }
}

/* ── GATEWAY SELECT ── */
void ui_draw_gateway_select(void)
{
    GatewayId gw = g_combat.gateway;
    static const char *gw_labels[] = {
        "Human Gateway", "Monster Portal", "Dark Rift"
    };
    static const char *gw_flavor[] = {
        "20 floors of human warriors – main story path.",
        "10 floors of monsters – optional challenge.",
        "5 floors of darkness – post-game ultimate challenge."
    };

    int cx = SCREEN_W / 2;
    DrawText(gw_labels[gw],
        cx - MeasureText(gw_labels[gw], 32)/2, 30, 32, C_GOLD);
    DrawText(gw_flavor[gw],
        cx - MeasureText(gw_flavor[gw], 16)/2, 74, 16, C_DIM);

    int prog = g_player.gw_progress[gw];
    DrawText(TextFormat("Progress: %d / %d floors", prog, GATEWAY_DEPTH),
        cx - 120, 104, 18, C_WHITE);

    /* Floor list */
    for (int f = 0; f < GATEWAY_DEPTH; f++) {
        int enemy_id = g_gw_enemies[gw][f];
        const EnemyDef *e = &g_enemies[enemy_id];
        bool completed = (f < prog);
        bool current   = (f == prog);
        bool locked    = (f > prog);

        Rectangle row = { (float)(cx - 260), (float)(148 + f * 88), 520, 76 };
        Color border   = completed ? C_GREEN : (current ? C_GOLD : C_BORDER);
        ui_panel(row, C_PANEL, border);

        DrawText(TextFormat("Floor %d: %s", f + 1, e->name),
            cx - 244, (int)row.y + 10, 20,
            completed ? C_GREEN : (current ? C_WHITE : C_DIM));
        DrawText(TextFormat("HP: %d  STR: %d  SPD: %d  XP: %d  Gold: %d",
            e->base_stats.max_life, e->base_stats.strength,
            e->base_stats.speed, e->xp_reward, e->gold_reward),
            cx - 244, (int)row.y + 38, 14,
            locked ? C_DIM : C_DIM);

        if (completed) {
            DrawText("CLEARED", (int)row.x + (int)row.width - 100, (int)row.y + 26, 18, C_GREEN);
        } else if (current) {
            Rectangle fb = { row.x + row.width - 140, row.y + 14, 130, 44 };
            if (ui_button(fb, "Enter!", true)) {
                combat_start(gw, f);
                g_scene = SCENE_COMBAT;
            }
        } else {
            DrawText("LOCKED", (int)row.x + (int)row.width - 90, (int)row.y + 26, 18, C_DIM);
        }
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) g_scene = SCENE_HUB;
}

/* ── GAME OVER ── */
void ui_draw_game_over(void)
{
    int cx = SCREEN_W / 2;
    DrawText("GAME OVER",
        cx - MeasureText("GAME OVER", 60)/2, 160, 60, C_RED);
    DrawText(TextFormat("You fell as a Level %d %s.", g_player.level, class_name(g_player.pc)),
        cx - 180, 250, 22, C_WHITE);
    DrawText(TextFormat("Gold earned: %d", g_player.gold),
        cx - 80, 286, 18, C_GOLD);

    Rectangle replay = { (float)(cx - 110), 360, 220, 54 };
    if (ui_button(replay, "Play Again", true)) {
        g_scene    = SCENE_TITLE;
        s_naming   = false;
        s_name_len = 4;
        memcpy(s_name_buf, "Hero", 5);
    }
}

/* ── VICTORY ── */
void ui_draw_victory(void)
{
    int cx = SCREEN_W / 2;
    DrawText("VICTORY!",
        cx - MeasureText("VICTORY!", 60)/2, 100, 60, C_GOLD);
    DrawText("You have conquered the Dark Rift!",
        cx - MeasureText("You have conquered the Dark Rift!", 24)/2, 176, 24, C_WHITE);
    DrawText("Warlord Baka has been defeated. Peace returns to the land.",
        cx - MeasureText("Warlord Baka has been defeated. Peace returns to the land.", 16)/2,
        214, 16, C_DIM);

    DrawText(TextFormat("Final Level: %d", g_player.level), cx - 100, 270, 20, C_WHITE);
    DrawText(TextFormat("Gold: %d", g_player.gold),          cx - 100, 296, 20, C_GOLD);

    Rectangle hub_btn  = { (float)(cx - 230), 380, 200, 54 };
    Rectangle play_btn = { (float)(cx + 30),  380, 200, 54 };
    if (ui_button(hub_btn,  "Return to Hub",  true)) g_scene = SCENE_HUB;
    if (ui_button(play_btn, "Play Again",     true)) {
        g_scene    = SCENE_TITLE;
        s_naming   = false;
        s_name_len = 4;
        memcpy(s_name_buf, "Hero", 5);
    }
}
