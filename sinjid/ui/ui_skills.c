#include "ui_internal.h"

void ui_draw_skills(void)
{
    txt(20, 16, 26, C_GOLD, "Skill Master");
    txt(20, 50, 19, C_WHITE, TextFormat("Skill Points: %d", g_ctx.player.skill_points));
    txt(300, 50, 18, C_DIM, TextFormat("Class: %s", class_name(g_ctx.player.pc)));

    const SkillDef *skills = skills_for_class(g_ctx.player.pc);
    for (int i = 0; i < MAX_SKILLS; i++) {
        const SkillDef *sk = &skills[i];
        int slvl = g_ctx.player.skill_level[i];
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

        bool can = (g_ctx.player.skill_points > 0 && slvl < sk->max_level);
        Rectangle up = { (float)(SCREEN_W - 170), row.y + 28, 140, 44 };
        if (ui_button(up, "Upgrade", can)) {
            g_ctx.player.skill_level[i]++;
            g_ctx.player.skill_points--;
            g_ctx.scene_dirty = true;
        }
    }
    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) anim_fade_to(SCENE_HUB);
}
