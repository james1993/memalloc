#include "combat.h"
#include "player.h"
#include "data.h"
#include "events.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define EVT(type, value, for_player) \
    evt_push(&g_ctx.events, (type), (value), (for_player))

// ── Forward declarations ──────────────────────────────────────────────────
extern const int g_consume_effect_count;

// ── Helpers ───────────────────────────────────────────────────────────────

void combat_log(const char *fmt, ...)
{
    CombatState *cs = &g_ctx.combat;
    if (cs->log_count >= MAX_COMBAT_LOG) {
        // Shift entries up
        memmove(cs->log[0], cs->log[1],
                (MAX_COMBAT_LOG - 1) * COMBAT_LOG_LEN);
        cs->log_count = MAX_COMBAT_LOG - 1;
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cs->log[cs->log_count], COMBAT_LOG_LEN, fmt, ap);
    va_end(ap);
    cs->log_count++;
}

static int rand_range(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + rand() % (hi - lo + 1);
}

// Compute physical damage: attacker_str * pct/100 ± 10%, minus defender_def
static int calc_damage(int attacker_str, int pct, int defender_def)
{
    int base = attacker_str * pct / 100;
    // ±10% variance
    int var = MAX(1, base / 10);
    base += rand_range(-var, var);
    base -= defender_def;
    return MAX(1, base);
}

// Apply a status to the enemy (overrides if stronger)
static void apply_enemy_status(StatusFlags flag, int turns)
{
    g_ctx.combat.enemy.status       |= flag;
    g_ctx.combat.enemy.status_turns  = turns;
}

// Apply a status to the player
static void apply_player_status(StatusFlags flag, int turns)
{
    g_ctx.player.status       |= flag;
    g_ctx.player.status_turns  = turns;
}

// Tick enemy status effects; returns damage dealt to enemy
static int enemy_tick_status(void)
{
    int dmg = 0;
    EnemyDef *e = &g_ctx.combat.enemy;
    if (e->status & STATUS_POISONED) {
        dmg = 3;
        e->status_turns--;
        if (e->status_turns <= 0) e->status &= ~STATUS_POISONED;
    }
    return dmg;
}

// ── Combat start ──────────────────────────────────────────────────────────

void combat_start(GatewayId gw, int floor_idx)
{
    srand((unsigned)time(NULL));
    CombatState *cs = &g_ctx.combat;
    memset(cs, 0, sizeof(*cs));

    cs->gateway = gw;
    cs->floor   = floor_idx;

    int enemy_id = g_gw_enemies[gw][floor_idx];
    cs->enemy = g_enemies[enemy_id];
    cs->enemy.current_life = cs->enemy.base_stats.max_life;
    cs->enemy.current_mana = cs->enemy.base_stats.max_mana;
    cs->enemy.status       = STATUS_NONE;
    cs->enemy.status_turns = 0;

    cs->str_bonus      = 0;
    cs->spd_bonus      = 0;
    cs->def_bonus      = 0;
    cs->enemy_ai_delay = 0.6f;

    // Decide who goes first based on speed
    cs->player_turn = (player_effective_spd(&g_ctx.player, &g_ctx.combat) >= cs->enemy.base_stats.speed);

    combat_log("Battle starts: %s", cs->enemy.name);
    if (!cs->player_turn)
        combat_log("%s moves first!", cs->enemy.name);
}

// ── Check over ────────────────────────────────────────────────────────────

bool combat_is_over(void)
{
    return g_ctx.combat.combat_over;
}

// ── Player turn actions ───────────────────────────────────────────────────

static void end_player_turn(void)
{
    // Tick player status
    int pdmg = player_tick_status(&g_ctx.player);
    if (pdmg > 0) {
        g_ctx.player.current_life -= pdmg;
        combat_log("Poison deals %d to you!", pdmg);
    }
    if (g_ctx.player.current_life <= 0) {
        g_ctx.player.current_life = 0;
        g_ctx.combat.combat_over = true;
        g_ctx.combat.player_won  = false;
        combat_log("You have been defeated...");
        return;
    }
    g_ctx.combat.player_turn = false;
}

bool combat_action_attack(void)
{
    if (!g_ctx.combat.player_turn || g_ctx.combat.combat_over) return false;
    if (g_ctx.player.status & STATUS_STUNNED) {
        g_ctx.player.status &= ~STATUS_STUNNED;
        combat_log("You are stunned and lose your turn!");
        end_player_turn();
        return true;
    }

    EnemyDef *e = &g_ctx.combat.enemy;
    if (e->status & STATUS_DODGING) {
        e->status &= ~STATUS_DODGING;
        combat_log("%s dodges your attack!", e->name);
        end_player_turn();
        return true;
    }

    int dmg = calc_damage(player_effective_str(&g_ctx.player, &g_ctx.combat), 100, e->base_stats.defense);
    if (e->status & STATUS_SHIELDED) {
        dmg = dmg * 70 / 100;
        e->status &= ~STATUS_SHIELDED;
    }
    e->current_life -= dmg;
    EVT(EVT_HIT_ENEMY, dmg, false);
    combat_log("You attack for %d damage.", dmg);

    // Tick enemy poison
    int edot = enemy_tick_status();
    if (edot > 0) {
        e->current_life -= edot;
        EVT(EVT_POISON_TICK, edot, false);
        combat_log("Poison deals %d to %s!", edot, e->name);
    }

    if (e->current_life <= 0) {
        e->current_life = 0;
        g_ctx.combat.combat_over = true;
        g_ctx.combat.player_won  = true;
        combat_log("You defeated %s!", e->name);
        combat_apply_rewards();
        EVT(EVT_COMBAT_WIN, 0, false);
        return true;
    }
    end_player_turn();
    return true;
}

bool combat_action_skill(int skill_idx)
{
    if (!g_ctx.combat.player_turn || g_ctx.combat.combat_over) return false;
    if (skill_idx < 0 || skill_idx >= MAX_SKILLS) return false;

    const SkillDef *sk = &skills_for_class(g_ctx.player.pc)[skill_idx];
    int slvl = g_ctx.player.skill_level[skill_idx];    // 1..5
    int cost  = sk->mana_cost + (slvl - 1) * 2;   // slight increase per level
    if (g_ctx.player.current_mana < cost) {
        combat_log("Not enough mana! (need %d)", cost);
        return false;
    }
    if (g_ctx.player.status & STATUS_STUNNED) {
        g_ctx.player.status &= ~STATUS_STUNNED;
        combat_log("You are stunned and lose your turn!");
        end_player_turn();
        return true;
    }

    g_ctx.player.current_mana -= cost;
    EnemyDef *e = &g_ctx.combat.enemy;

    // Scale effect with skill level
    int scaled_val = sk->base_value + (slvl - 1) * (sk->base_value / 5);

    switch (sk->effect) {
        case SKILL_EFFECT_DAMAGE: {
            if (e->status & STATUS_DODGING) {
                e->status &= ~STATUS_DODGING;
                combat_log("%s dodges %s!", e->name, sk->name);
                break;
            }
            int dmg = calc_damage(player_effective_str(&g_ctx.player, &g_ctx.combat), scaled_val,
                                  e->base_stats.defense);
            if (e->status & STATUS_SHIELDED) {
                dmg = dmg * 70 / 100;
                e->status &= ~STATUS_SHIELDED;
            }
            e->current_life -= dmg;
            EVT(EVT_HIT_ENEMY, dmg, false);
            EVT(EVT_SKILL_USE, 0, true);
            combat_log("%s hits for %d!", sk->name, dmg);
            if (sk->apply_status && sk->apply_status != STATUS_NONE) {
                apply_enemy_status(sk->apply_status, 3);
                combat_log("%s is now %s!",
                    e->name,
                    (sk->apply_status == STATUS_SLOWED) ? "slowed" : "affected");
            }
            break;
        }
        case SKILL_EFFECT_MULTI_HIT: {
            int total = 0;
            int hits  = MAX(1, sk->hits);
            for (int h = 0; h < hits; h++) {
                if (e->current_life <= 0) break;
                int dmg = calc_damage(player_effective_str(&g_ctx.player, &g_ctx.combat), scaled_val,
                                      e->base_stats.defense);
                e->current_life -= dmg;
                total += dmg;
            }
            EVT(EVT_HIT_ENEMY, total, false);
            EVT(EVT_SKILL_USE, 0, true);
            combat_log("%s hits %d times for %d total!", sk->name, hits, total);
            break;
        }
        case SKILL_EFFECT_HEAL: {
            int hp = scaled_val + (slvl - 1) * 10;
            g_ctx.player.current_life = MIN(g_ctx.player.current_life + hp,
                                        player_effective_life(&g_ctx.player));
            EVT(EVT_HEAL_PLAYER, hp, true);
            EVT(EVT_SKILL_USE, 0, true);
            combat_log("%s restores %d life.", sk->name, hp);
            break;
        }
        case SKILL_EFFECT_BUFF: {
            int delta = sk->base_value + slvl;
            if (sk->apply_status == STATUS_POWERED) {
                g_ctx.combat.str_bonus += delta;
                combat_log("Strength +%d for this battle!", delta);
            } else if (sk->apply_status == STATUS_SHIELDED) {
                g_ctx.combat.def_bonus += delta;
                apply_player_status(STATUS_SHIELDED, 3);
                combat_log("Defense +%d for 3 turns!", delta);
            }
            break;
        }
        case SKILL_EFFECT_DEBUFF: {
            int delta = sk->base_value + slvl;
            apply_enemy_status(STATUS_SLOWED, 3);
            e->base_stats.speed = MAX(1, e->base_stats.speed - delta);
            combat_log("%s's speed reduced by %d!", e->name, delta);
            break;
        }
        case SKILL_EFFECT_DOT: {
            apply_enemy_status(STATUS_POISONED, 3 + slvl);
            combat_log("%s is poisoned!", e->name);
            break;
        }
        case SKILL_EFFECT_STUN: {
            int dmg = calc_damage(player_effective_str(&g_ctx.player, &g_ctx.combat), scaled_val,
                                  e->base_stats.defense);
            e->current_life -= dmg;
            apply_enemy_status(STATUS_STUNNED, 1);
            combat_log("%s for %d + stunned!", sk->name, dmg);
            break;
        }
        case SKILL_EFFECT_DODGE: {
            apply_player_status(sk->apply_status, 1);
            combat_log("You prepare to dodge the next attack.");
            break;
        }
    }

    // Tick enemy poison
    int edot = enemy_tick_status();
    if (edot > 0 && e->current_life > 0) {
        e->current_life -= edot;
        EVT(EVT_POISON_TICK, edot, false);
        combat_log("Poison deals %d to %s!", edot, e->name);
    }

    if (e->current_life <= 0) {
        e->current_life = 0;
        g_ctx.combat.combat_over = true;
        g_ctx.combat.player_won  = true;
        combat_log("You defeated %s!", e->name);
        combat_apply_rewards();
        EVT(EVT_COMBAT_WIN, 0, false);
        return true;
    }
    end_player_turn();
    return true;
}

bool combat_action_use_item(int item_id)
{
    if (!g_ctx.combat.player_turn || g_ctx.combat.combat_over) return false;
    if (!player_use_consumable(&g_ctx.player, item_id)) {
        combat_log("Can't use that item.");
        return false;
    }
    combat_log("Used %s.", g_items[item_id].name);
    end_player_turn();
    return true;
}

bool combat_action_flee(void)
{
    if (!g_ctx.combat.player_turn || g_ctx.combat.combat_over) return false;
    // 50% chance to flee
    if (rand() % 2 == 0) {
        combat_log("You successfully fled!");
        g_ctx.combat.combat_over = true;
        g_ctx.combat.fled        = true;
    } else {
        combat_log("Couldn't escape!");
        end_player_turn();
    }
    return true;
}

// ── Enemy turn ────────────────────────────────────────────────────────────

void combat_enemy_turn(void)
{
    if (g_ctx.combat.combat_over || g_ctx.combat.player_turn) return;

    EnemyDef *e = &g_ctx.combat.enemy;

    // Tick enemy status
    if (e->status & STATUS_STUNNED) {
        e->status &= ~STATUS_STUNNED;
        combat_log("%s is stunned and loses their turn!", e->name);
        g_ctx.combat.player_turn = true;
        return;
    }
    int edot = enemy_tick_status();
    if (edot > 0) {
        e->current_life -= edot;
        combat_log("Poison deals %d to %s!", edot, e->name);
        if (e->current_life <= 0) {
            e->current_life      = 0;
            g_ctx.combat.combat_over = true;
            g_ctx.combat.player_won  = true;
            combat_log("You defeated %s!", e->name);
            combat_apply_rewards();
            return;
        }
    }

    // Choose action: 30% skill if available, else basic attack
    int action = 0; // 0 = basic attack
    if (e->skill_ids[0] != -1 && rand() % 10 < 3) {
        // Pick a random available skill
        int sk_count = 0;
        while (sk_count < 3 && e->skill_ids[sk_count] != -1) sk_count++;
        action = e->skill_ids[rand() % sk_count] + 1; // +1 to distinguish from 0
    }

    if (action == 0) {
        // Basic attack
        if (g_ctx.player.status & STATUS_DODGING) {
            g_ctx.player.status &= ~STATUS_DODGING;
            combat_log("You dodge %s's attack!", e->name);
        } else if (g_ctx.player.status & STATUS_SHIELDED) {
            int dmg = calc_damage(e->base_stats.strength, 100,
                                  player_effective_def(&g_ctx.player, &g_ctx.combat));
            dmg = dmg * 70 / 100;
            g_ctx.player.status_turns--;
            if (g_ctx.player.status_turns <= 0) g_ctx.player.status &= ~STATUS_SHIELDED;
            g_ctx.player.current_life -= dmg;
            EVT(EVT_HIT_PLAYER, dmg, true);
            combat_log("%s attacks for %d (shielded).", e->name, dmg);
        } else {
            int dmg = calc_damage(e->base_stats.strength, 100,
                                  player_effective_def(&g_ctx.player, &g_ctx.combat));
            g_ctx.player.current_life -= dmg;
            EVT(EVT_HIT_PLAYER, dmg, true);
            combat_log("%s attacks for %d.", e->name, dmg);
        }
    } else {
        // Enemy skill
        int sk_id = action - 1;
        const EnemySkill *sk = &g_enemy_skill_table[sk_id];
        switch (sk->effect) {
            case SKILL_EFFECT_DAMAGE: {
                if (g_ctx.player.status & STATUS_DODGING) {
                    g_ctx.player.status &= ~STATUS_DODGING;
                    combat_log("You dodge %s's %s!", e->name, sk->name);
                } else {
                    int dmg = calc_damage(e->base_stats.strength, sk->value,
                                          player_effective_def(&g_ctx.player, &g_ctx.combat));
                    if (g_ctx.player.status & STATUS_SHIELDED) {
                        dmg = dmg * 70 / 100;
                        g_ctx.player.status_turns--;
                        if (g_ctx.player.status_turns <= 0)
                            g_ctx.player.status &= ~STATUS_SHIELDED;
                    }
                    g_ctx.player.current_life -= dmg;
                    combat_log("%s uses %s for %d!", e->name, sk->name, dmg);
                }
                break;
            }
            case SKILL_EFFECT_BUFF: {
                e->base_stats.strength += sk->value;
                combat_log("%s uses %s! +%d strength.", e->name, sk->name, sk->value);
                break;
            }
            case SKILL_EFFECT_DEBUFF: {
                g_ctx.combat.spd_bonus -= sk->value;
                combat_log("%s uses %s! Your speed is reduced.", e->name, sk->name);
                break;
            }
            case SKILL_EFFECT_DOT: {
                apply_player_status(STATUS_POISONED, 3);
                combat_log("%s uses %s! You are poisoned!", e->name, sk->name);
                break;
            }
            case SKILL_EFFECT_STUN: {
                int dmg = calc_damage(e->base_stats.strength, sk->value,
                                      player_effective_def(&g_ctx.player, &g_ctx.combat));
                g_ctx.player.current_life -= dmg;
                apply_player_status(STATUS_STUNNED, 1);
                combat_log("%s uses %s for %d + stun!", e->name, sk->name, dmg);
                break;
            }
            case SKILL_EFFECT_HEAL: {
                e->current_life = MIN(e->current_life + sk->value,
                                      e->base_stats.max_life);
                combat_log("%s uses %s, healing %d!", e->name, sk->name, sk->value);
                break;
            }
            default: break;
        }
    }

    if (g_ctx.player.current_life <= 0) {
        g_ctx.player.current_life = 0;
        g_ctx.combat.combat_over  = true;
        g_ctx.combat.player_won   = false;
        EVT(EVT_COMBAT_LOSE, 0, true);
        combat_log("You have been defeated...");
        return;
    }
    g_ctx.combat.player_turn = true;
}

// ── Rewards ───────────────────────────────────────────────────────────────

void combat_apply_rewards(void)
{
    if (!g_ctx.combat.player_won) return;
    EnemyDef *e  = &g_ctx.combat.enemy;
    g_ctx.player.gold += e->gold_reward;
    bool levelled  = player_add_xp(&g_ctx.player, e->xp_reward);
    combat_log("Gained %d XP, %d gold.", e->xp_reward, e->gold_reward);
    if (levelled) {
        combat_log("LEVEL UP! Now level %d!", g_ctx.player.level);
        EVT(EVT_LEVEL_UP, g_ctx.player.level, true);
    }
    // Advance gateway progress
    GatewayId gw  = g_ctx.combat.gateway;
    int floor_idx = g_ctx.combat.floor;
    if (floor_idx >= g_ctx.player.gw_progress[gw])
        g_ctx.player.gw_progress[gw] = floor_idx + 1;
    if (g_ctx.player.gw_progress[gw] >= GATEWAY_DEPTH)
        g_ctx.player.gw_complete[gw] = true;
}
