#include "ui_internal.h"

/* Shared with ui_title.c via extern — defined here */
char        s_name_buf[32]      = "Hero";
int         s_name_len          = 4;
bool        s_naming            = false;
PlayerClass s_chosen_class      = CLASS_WARRIOR;

void ui_draw_class_select(void)
{
    int cx = SCREEN_W / 2;
    txt(cx - txt_w("Choose Your Class", 32)/2, 40, 32, C_GOLD, "Choose Your Class");

    static const struct {
        PlayerClass pc; const char *name; const char *desc; Color col;
    } classes[] = {
        { CLASS_WARRIOR, "Warrior", "High STR & DEF. Buffs and stuns.",     (Color){180,80,80,255} },
        { CLASS_ROGUE,   "Rogue",   "High SPD. Poison, dodge, multi-hit.",   (Color){80,180,80,255} },
        { CLASS_MAGE,    "Mage",    "High mana. Devastating spells.",         (Color){80,80,200,255} },
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

            Vector2 m = game_mouse();
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
            player_init(&g_ctx.player, s_name_buf, s_chosen_class);
            anim_combat_reset();
            s_naming = false;
            anim_fade_to(SCENE_HUB);
        }
        Rectangle back2 = { cx - 100, 400, 200, 40 };
        if (ui_button(back2, "<- Back", true)) s_naming = false;
    }
}
