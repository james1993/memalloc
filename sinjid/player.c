#include "player.h"
#include "data.h"
#include <string.h>
#include <stdio.h>

const char *class_name(PlayerClass pc)
{
    switch (pc) {
        case CLASS_WARRIOR: return "Warrior";
        case CLASS_ROGUE:   return "Rogue";
        case CLASS_MAGE:    return "Mage";
        default:            return "Unknown";
    }
}

Stats class_base_stats(PlayerClass pc)
{
    switch (pc) {
        case CLASS_WARRIOR: return (Stats){ 120, 40, 12, 8,  3 };
        case CLASS_ROGUE:   return (Stats){  90, 50,  9,13,  1 };
        case CLASS_MAGE:    return (Stats){  80,100,  6,10,  0 };
        default:            return (Stats){ 100, 60, 10,10,  2 };
    }
}

int xp_for_level(int level)
{
    // Roughly exponential curve
    return 30 + level * 25 + level * level * 5;
}

void player_init(Player *p, const char *name, PlayerClass pc)
{
    memset(p, 0, sizeof(*p));
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->pc           = pc;
    p->level        = 1;
    p->xp           = 0;
    p->xp_to_next   = xp_for_level(1);
    p->gold         = 80;
    p->stat_points  = 0;
    p->skill_points = 0;
    p->base         = class_base_stats(pc);
    p->current_life = p->base.max_life;
    p->current_mana = p->base.max_mana;

    for (int i = 0; i < EQUIP_SLOT_COUNT; i++)
        p->equip[i] = -1;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        p->bag_ids[i] = -1;
        p->bag_qty[i] = 0;
    }
    for (int i = 0; i < MAX_SKILLS; i++)
        p->skill_level[i] = 1;

    // Start with a small potion
    player_add_item(p, 23);
}

// ── Equipment stat helpers ────────────────────────────────────────────────

static int equip_bonus(const Player *p, EquipSlot slot, int field)
{
    int id = p->equip[slot];
    if (id < 0 || id >= g_items_count) return 0;
    const ItemDef *it = &g_items[id];
    switch (field) {
        case 0: return it->bonus_life;
        case 1: return it->bonus_mana;
        case 2: return it->bonus_str;
        case 3: return it->bonus_spd;
        case 4: return it->bonus_def;
    }
    return 0;
}

static int total_equip_bonus(const Player *p, int field)
{
    int total = 0;
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++)
        total += equip_bonus(p, s, field);
    return total;
}

int player_effective_life(const Player *p)
{
    return p->base.max_life + total_equip_bonus(p, 0);
}
int player_effective_mana(const Player *p)
{
    return p->base.max_mana + total_equip_bonus(p, 1);
}
int player_effective_str(const Player *p, const CombatState *cs)
{
    int bonus = cs ? cs->str_bonus : 0;
    int v = p->base.strength + total_equip_bonus(p, 2) + bonus;
    return MAX(1, v);
}
int player_effective_spd(const Player *p, const CombatState *cs)
{
    int bonus = cs ? cs->spd_bonus : 0;
    int v = p->base.speed + total_equip_bonus(p, 3) + bonus;
    return MAX(1, v);
}
int player_effective_def(const Player *p, const CombatState *cs)
{
    int bonus = cs ? cs->def_bonus : 0;
    int v = p->base.defense + total_equip_bonus(p, 4) + bonus;
    return MAX(0, v);
}

// ── Leveling ──────────────────────────────────────────────────────────────

bool player_add_xp(Player *p, int xp)
{
    bool levelled = false;
    p->xp += xp;
    while (p->xp >= p->xp_to_next && p->level < 50) {
        p->xp       -= p->xp_to_next;
        p->level    += 1;
        p->xp_to_next = xp_for_level(p->level);
        p->stat_points  += 2;
        p->skill_points += 1;
        levelled = true;
    }
    return levelled;
}

// ── Inventory ─────────────────────────────────────────────────────────────

bool player_add_item(Player *p, int item_id)
{
    // Stack consumables
    if (g_items[item_id].type == ITEM_CONSUMABLE) {
        for (int i = 0; i < MAX_INVENTORY; i++) {
            if (p->bag_ids[i] == item_id) {
                p->bag_qty[i]++;
                return true;
            }
        }
    }
    // Find empty slot
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (p->bag_ids[i] == -1) {
            p->bag_ids[i] = item_id;
            p->bag_qty[i] = 1;
            return true;
        }
    }
    return false; // bag full
}

bool player_remove_item(Player *p, int item_id)
{
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (p->bag_ids[i] == item_id) {
            p->bag_qty[i]--;
            if (p->bag_qty[i] <= 0) {
                p->bag_ids[i] = -1;
                p->bag_qty[i] = 0;
            }
            return true;
        }
    }
    return false;
}

void player_equip(Player *p, int item_id)
{
    if (item_id < 0 || item_id >= g_items_count) return;
    const ItemDef *it = &g_items[item_id];
    EquipSlot slot;
    switch (it->type) {
        case ITEM_WEAPON:    slot = EQUIP_WEAPON;    break;
        case ITEM_ARMOR:     slot = EQUIP_ARMOR;     break;
        case ITEM_ACCESSORY: slot = EQUIP_ACCESSORY; break;
        default: return;
    }
    // Remove from bag
    player_remove_item(p, item_id);
    // Put old equipped item back in bag
    if (p->equip[slot] != -1)
        player_add_item(p, p->equip[slot]);
    p->equip[slot] = item_id;
    // Clamp current life/mana to new max
    int max_life = player_effective_life(p);
    int max_mana = player_effective_mana(p);
    if (p->current_life > max_life) p->current_life = max_life;
    if (p->current_mana > max_mana) p->current_mana = max_mana;
}

void player_unequip(Player *p, EquipSlot slot)
{
    if (p->equip[slot] == -1) return;
    player_add_item(p, p->equip[slot]);
    p->equip[slot] = -1;
}

bool player_use_consumable(Player *p, int item_id)
{
    if (item_id < 0 || item_id >= g_items_count) return false;
    if (g_items[item_id].type != ITEM_CONSUMABLE) return false;
    if (!player_remove_item(p, item_id)) return false;

    extern const int g_consume_effect_count;

    if (item_id < g_consume_effect_count) {
        p->current_life = MIN(p->current_life + g_consume_effect[item_id].life,
                              player_effective_life(p));
        p->current_mana = MIN(p->current_mana + g_consume_effect[item_id].mana,
                              player_effective_mana(p));
    }
    return true;
}

void player_rest(Player *p)
{
    p->current_life = player_effective_life(p);
    p->current_mana = player_effective_mana(p);
    p->status       = STATUS_NONE;
    p->status_turns = 0;
}

int player_tick_status(Player *p)
{
    int dmg = 0;
    if (p->status & STATUS_POISONED) {
        dmg = 3;
        p->status_turns--;
        if (p->status_turns <= 0)
            p->status &= ~STATUS_POISONED;
    }
    if (p->status & STATUS_STUNNED) {
        p->status &= ~STATUS_STUNNED;
        // caller checks STUNNED to skip turn
    }
    return dmg;
}
