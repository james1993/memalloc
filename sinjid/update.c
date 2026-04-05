#include "update.h"
#include "game.h"
#include "combat.h"
#include "anim.h"

/* ── Combat update ───────────────────────────────────────────────────────
   Runs the enemy AI timer entirely in game-logic space.
   Events are pushed to g_ctx.events; ui_combat.c drains them during draw. */
static void combat_update(float dt)
{
    CombatState *cs = &g_ctx.combat;
    if (cs->player_turn || cs->combat_over) return;

    cs->enemy_ai_delay -= dt;
    if (cs->enemy_ai_delay <= 0.0f) {
        combat_enemy_turn();
        cs->enemy_ai_delay = 0.6f;
    }
}

/* ── Public dispatcher ───────────────────────────────────────────────── */
void scene_update(float dt)
{
    /* Tick hub flash timer regardless of scene so it expires correctly */
    if (g_ctx.hub_save_flash > 0.0f) {
        g_ctx.hub_save_flash -= dt;
        if (g_ctx.hub_save_flash < 0.0f) g_ctx.hub_save_flash = 0.0f;
    }

    switch (G_SCENE) {
        case SCENE_COMBAT: combat_update(dt); break;
        default:           break;
    }
}
