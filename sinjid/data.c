#include "data.h"
#include <raylib.h>

// ─────────────────────────────────────────────
//  Skill definitions
// ─────────────────────────────────────────────

// Warrior – strength-focused physical fighter
const SkillDef g_warrior_skills[MAX_SKILLS] = {
    {
        "Power Strike",
        "A devastating blow dealing 150% strength damage.",
        SKILL_EFFECT_DAMAGE, 150, STATUS_NONE, 8, 1, 1, 5
    },
    {
        "Battle Cry",
        "Boosts your strength for 3 turns.",
        SKILL_EFFECT_BUFF, 6, STATUS_POWERED, 10, 1, 1, 5
    },
    {
        "Shield Bash",
        "A stunning strike that skips the enemy's next turn.",
        SKILL_EFFECT_STUN, 80, STATUS_STUNNED, 12, 1, 1, 5
    },
    {
        "Whirlwind",
        "Spin and strike twice in quick succession.",
        SKILL_EFFECT_MULTI_HIT, 70, STATUS_NONE, 14, 2, 1, 5
    },
    {
        "Fortify",
        "Brace yourself, raising defense for 3 turns.",
        SKILL_EFFECT_BUFF, 5, STATUS_SHIELDED, 10, 1, 1, 5
    },
};

// Rogue – speed-focused agile striker
const SkillDef g_rogue_skills[MAX_SKILLS] = {
    {
        "Backstab",
        "A swift hidden attack dealing speed-scaled damage.",
        SKILL_EFFECT_DAMAGE, 130, STATUS_NONE, 8, 1, 1, 5
    },
    {
        "Poison Blade",
        "Coats your blade with poison; enemy takes 3 dmg/turn.",
        SKILL_EFFECT_DOT, 20, STATUS_POISONED, 10, 1, 1, 5
    },
    {
        "Shadow Step",
        "Phase through shadow to dodge the next attack.",
        SKILL_EFFECT_DODGE, 0, STATUS_DODGING, 8, 1, 1, 5
    },
    {
        "Twin Strike",
        "Two rapid strikes in a single turn.",
        SKILL_EFFECT_MULTI_HIT, 65, STATUS_NONE, 12, 2, 1, 5
    },
    {
        "Smoke Bomb",
        "Blind the enemy, reducing their speed for 3 turns.",
        SKILL_EFFECT_DEBUFF, 4, STATUS_SLOWED, 10, 1, 1, 5
    },
};

// Mage – mana-heavy spell caster
const SkillDef g_mage_skills[MAX_SKILLS] = {
    {
        "Fireball",
        "Launch a ball of fire for high magic damage.",
        SKILL_EFFECT_DAMAGE, 160, STATUS_NONE, 12, 1, 1, 5
    },
    {
        "Ice Shard",
        "A shard of ice that slows the enemy.",
        SKILL_EFFECT_DAMAGE, 110, STATUS_SLOWED, 10, 1, 1, 5
    },
    {
        "Chain Lightning",
        "Unleash a bolt of lightning for massive damage.",
        SKILL_EFFECT_DAMAGE, 200, STATUS_NONE, 20, 1, 1, 5
    },
    {
        "Mana Shield",
        "Erect a barrier that absorbs the next attack.",
        SKILL_EFFECT_DODGE, 0, STATUS_SHIELDED, 10, 1, 1, 5
    },
    {
        "Heal",
        "Channel life energy to restore your health.",
        SKILL_EFFECT_HEAL, 40, STATUS_NONE, 14, 1, 1, 5
    },
};

const SkillDef *skills_for_class(PlayerClass pc)
{
    switch (pc) {
        case CLASS_WARRIOR: return g_warrior_skills;
        case CLASS_ROGUE:   return g_rogue_skills;
        case CLASS_MAGE:    return g_mage_skills;
        default:            return g_warrior_skills;
    }
}

// ─────────────────────────────────────────────
//  Item definitions
//  { name, type, +life, +mana, +str, +spd, +def, price }
// ─────────────────────────────────────────────
const ItemDef g_items[] = {
    // id 0 – placeholder/none
    { "Nothing",        ITEM_NONE,       0,  0, 0, 0, 0,   0 },

    // ── Weapons ──────────────────────────────
    // id 1
    { "Iron Knife",     ITEM_WEAPON,     0,  0, 2, 1, 0,  50 },
    // id 2
    { "Energy Knife",   ITEM_WEAPON,     0,  5, 3, 2, 0, 100 },
    // id 3
    { "Silver Knife",   ITEM_WEAPON,     0,  0, 4, 3, 0, 160 },
    // id 4
    { "Spiked Axe",     ITEM_WEAPON,     0,  0, 6, 0, 0, 200 },
    // id 5
    { "Heavy Blade",    ITEM_WEAPON,     0,  0, 8, 0, 1, 280 },
    // id 6
    { "Katana",         ITEM_WEAPON,     0,  0, 7, 2, 0, 320 },
    // id 7
    { "Shadow Katana",  ITEM_WEAPON,     0,  8,10, 3, 0, 500 },
    // id 8
    { "Fusion Edge",    ITEM_WEAPON,     0, 10,12, 2, 0, 700 },
    // id 9
    { "Ion Bo",         ITEM_WEAPON,     0, 15, 8, 5, 0, 650 },
    // id 10
    { "Golden Blade",   ITEM_WEAPON,     5,  5,14, 2, 2,1000 },

    // ── Armor ────────────────────────────────
    // id 11
    { "Leather Armour", ITEM_ARMOR,     10,  0, 0, 0, 1,  60 },
    // id 12
    { "Metal Plates",   ITEM_ARMOR,     15,  0, 0,-1, 3, 140 },
    // id 13
    { "Metal Armour",   ITEM_ARMOR,     20,  0, 1,-1, 4, 220 },
    // id 14
    { "Snake Skin",     ITEM_ARMOR,     12,  5, 0, 2, 2, 260 },
    // id 15
    { "Shadow Armour",  ITEM_ARMOR,     25, 10, 0, 1, 6, 480 },
    // id 16
    { "Golden Armour",  ITEM_ARMOR,     30,  5, 2, 0, 8, 800 },

    // ── Accessories ──────────────────────────
    // id 17
    { "Leather Wrist",  ITEM_ACCESSORY,  0,  0, 1, 1, 0,  40 },
    // id 18
    { "Metal Wrist",    ITEM_ACCESSORY,  0,  0, 2, 0, 1,  80 },
    // id 19
    { "Bandana",        ITEM_ACCESSORY,  5,  0, 0, 2, 0,  70 },
    // id 20
    { "Metal Band",     ITEM_ACCESSORY,  5,  0, 1, 1, 1, 130 },
    // id 21
    { "Focus Ring",     ITEM_ACCESSORY,  0, 15, 0, 2, 0, 200 },
    // id 22
    { "Warrior Ring",   ITEM_ACCESSORY,  0,  0, 4, 0, 2, 300 },

    // ── Consumables ──────────────────────────
    // id 23
    { "Small Potion",   ITEM_CONSUMABLE, 0,  0, 0, 0, 0,  30 },
    // id 24
    { "Large Potion",   ITEM_CONSUMABLE, 0,  0, 0, 0, 0,  80 },
    // id 25
    { "Mana Vial",      ITEM_CONSUMABLE, 0,  0, 0, 0, 0,  40 },
    // id 26
    { "Elixir",         ITEM_CONSUMABLE, 0,  0, 0, 0, 0, 150 },
};
const int g_items_count = sizeof(g_items) / sizeof(g_items[0]);

// Consumable heal amounts (life, mana) indexed by item id
// Only meaningful for ITEM_CONSUMABLE items
typedef struct { int life; int mana; } ConsumeEffect;
const ConsumeEffect g_consume_effect[] = {
    [23] = { 30,  0 },
    [24] = { 80,  0 },
    [25] = {  0, 40 },
    [26] = { 60, 40 },
};
const int g_consume_effect_count =
    (int)(sizeof(g_consume_effect) / sizeof(g_consume_effect[0]));

// ─────────────────────────────────────────────
//  Shop inventories
// ─────────────────────────────────────────────
const int g_shop_basic[]      = { 1, 11, 17, 19, 23, 24, 25 };
const int g_shop_basic_count  = 7;

const int g_shop_mid[]        = { 3, 4, 5, 13, 14, 18, 20, 21, 24, 25, 26 };
const int g_shop_mid_count    = 11;

const int g_shop_adv[]        = { 7, 8, 9, 10, 15, 16, 22, 26 };
const int g_shop_adv_count    = 8;

// ─────────────────────────────────────────────
//  Enemy definitions
//  skill_ids: indices into enemy skill table (-1 = unused)
//  We use a simple flat array of enemy "skill" effects.
// ─────────────────────────────────────────────
// Enemy skill table (reusable across enemies)
// Each entry: { effect, base_value, mana_cost }
typedef struct { SkillEffectType effect; int value; const char *name; } EnemySkill;

const EnemySkill g_enemy_skill_table[] = {
    /* 0 */ { SKILL_EFFECT_DAMAGE,  120, "Heavy Blow"    },
    /* 1 */ { SKILL_EFFECT_DOT,      15, "Poison Strike" },
    /* 2 */ { SKILL_EFFECT_DAMAGE,  180, "Power Slash"   },
    /* 3 */ { SKILL_EFFECT_BUFF,      5, "Rage"          },
    /* 4 */ { SKILL_EFFECT_DAMAGE,  150, "Fireball"      },
    /* 5 */ { SKILL_EFFECT_DEBUFF,    3, "Weaken"        },
    /* 6 */ { SKILL_EFFECT_DAMAGE,  200, "Dark Strike"   },
    /* 7 */ { SKILL_EFFECT_STUN,     60, "Bash"          },
    /* 8 */ { SKILL_EFFECT_DAMAGE,  250, "Shadow Slash"  },
    /* 9 */ { SKILL_EFFECT_HEAL,     30, "Mend"          },
};

//  Full enemy roster
//  { name, {max_life, max_mana, strength, speed, defense},
//    xp_reward, gold_reward, {skill_ids}, color }
const EnemyDef g_enemies[] = {
    // ── Human Gateway ────────────────────────────────────────────────────
    /* 00 */ { "Thief",
               { 40, 0, 5, 6, 0 }, 12, 15,
               { -1,-1,-1 }, (Color){180,120, 60,255} },
    /* 01 */ { "Bandit",
               { 55, 0, 7, 5, 1 }, 18, 22,
               { 0,-1,-1 }, (Color){150, 90, 50,255} },
    /* 02 */ { "Raider",
               { 70, 0, 9, 5, 1 }, 25, 30,
               { 0, 7,-1 }, (Color){160, 80, 40,255} },
    /* 03 */ { "Warrior",
               { 90, 20,11, 6, 2 }, 35, 42,
               { 0, 3,-1 }, (Color){100,100,180,255} },
    /* 04 */ { "Seer",
               { 60, 60, 8, 7, 0 }, 30, 38,
               { 4, 5,-1 }, (Color){120, 60,200,255} },
    /* 05 */ { "Mercenary",
               {100, 0,13, 6, 2 }, 45, 55,
               { 2,-1,-1 }, (Color){130,130, 70,255} },
    /* 06 */ { "Assailant",
               { 85, 10,12, 9, 1 }, 40, 50,
               { 0, 1,-1 }, (Color){ 80,160, 80,255} },
    /* 07 */ { "Shaman",
               { 75, 80,10, 7, 1 }, 50, 60,
               { 4, 9,-1 }, (Color){200, 60,200,255} },
    /* 08 */ { "Agent",
               {110, 20,14, 8, 2 }, 60, 70,
               { 2, 5,-1 }, (Color){ 60, 60, 60,255} },
    /* 09 */ { "Ronin",
               {120, 0,15, 9, 3 }, 70, 85,
               { 2, 3,-1 }, (Color){200,160, 60,255} },
    /* 10 */ { "Samurai",
               {150, 30,18,10, 4 }, 90,110,
               { 2, 3, 7}, (Color){180, 40, 40,255} },

    // ── Monster Portal ───────────────────────────────────────────────────
    /* 11 */ { "Skeleton",
               { 80, 0,10, 5, 2 }, 30, 35,
               { 0,-1,-1 }, (Color){220,220,200,255} },
    /* 12 */ { "Zombie",
               {100, 0,12, 3, 3 }, 38, 42,
               { 0, 7,-1 }, (Color){ 80,160, 80,255} },
    /* 13 */ { "Goblin",
               { 70,10, 9, 9, 0 }, 32, 38,
               { 1,-1,-1 }, (Color){ 60,180, 60,255} },
    /* 14 */ { "Troll",
               {160, 0,16, 4, 4 }, 55, 65,
               { 3,-1,-1 }, (Color){ 80,120, 60,255} },
    /* 15 */ { "Blood Spirit",
               { 90,100,14,10, 1 }, 70, 80,
               { 4, 6,-1 }, (Color){200, 30, 30,255} },

    // ── Dark Rift ─────────────────────────────────────────────────────────
    /* 16 */ { "Shadow Guard",
               {180, 40,20,10, 5 }, 100,120,
               { 6, 3,-1 }, (Color){ 40, 40, 80,255} },
    /* 17 */ { "Void Crawler",
               {160, 60,18,12, 4 }, 110,130,
               { 6, 5,-1 }, (Color){ 60, 20, 80,255} },
    /* 18 */ { "Dark Knight",
               {220, 30,24,10, 7 }, 130,160,
               { 8, 3, 7}, (Color){ 20, 20, 50,255} },
    /* 19 */ { "Soul Reaper",
               {200,100,22,13, 5 }, 150,180,
               { 8, 6,-1 }, (Color){ 80,  0,120,255} },
    /* 20 */ { "Warlord Baka",
               {300, 60,28,12, 8 }, 250,300,
               { 8, 3, 7}, (Color){180, 10, 10,255} },
};
const int g_enemies_count = sizeof(g_enemies)/sizeof(g_enemies[0]);

// gateway → floor → enemy index
const int g_gw_enemies[NUM_GATEWAYS][GATEWAY_DEPTH] = {
    // Human Gateway (floors 0-4 → easy → hard)
    { 0, 2, 4, 7, 10 },
    // Monster Portal
    { 11, 12, 13, 14, 15 },
    // Dark Rift
    { 16, 17, 18, 19, 20 },
};
