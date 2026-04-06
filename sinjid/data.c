#include "data.h"
#include <raylib.h>
#include <string.h>

// ─────────────────────────────────────────────
//  Skill definitions (mutable; may be overridden by data_load_files)
// ─────────────────────────────────────────────

SkillDef g_warrior_skills[MAX_SKILLS];
SkillDef g_rogue_skills[MAX_SKILLS];
SkillDef g_mage_skills[MAX_SKILLS];

static void init_warrior_skills(void)
{
    SkillDef s[] = {
        { "Power Strike",  "A devastating blow dealing 150% strength damage.",
          SKILL_EFFECT_DAMAGE, 150, STATUS_NONE, 8, 1, 1, 5 },
        { "Battle Cry",    "Boosts your strength for 3 turns.",
          SKILL_EFFECT_BUFF, 6, STATUS_POWERED, 10, 1, 1, 5 },
        { "Shield Bash",   "A stunning strike that skips the enemy's next turn.",
          SKILL_EFFECT_STUN, 80, STATUS_STUNNED, 12, 1, 1, 5 },
        { "Whirlwind",     "Spin and strike twice in quick succession.",
          SKILL_EFFECT_MULTI_HIT, 70, STATUS_NONE, 14, 2, 1, 5 },
        { "Fortify",       "Brace yourself, raising defense for 3 turns.",
          SKILL_EFFECT_BUFF, 5, STATUS_SHIELDED, 10, 1, 1, 5 },
    };
    memcpy(g_warrior_skills, s, sizeof(s));
}

static void init_rogue_skills(void)
{
    SkillDef s[] = {
        { "Backstab",    "A swift hidden attack dealing speed-scaled damage.",
          SKILL_EFFECT_DAMAGE, 130, STATUS_NONE, 8, 1, 1, 5 },
        { "Poison Blade","Coats your blade with poison; enemy takes 3 dmg/turn.",
          SKILL_EFFECT_DOT, 20, STATUS_POISONED, 10, 1, 1, 5 },
        { "Shadow Step", "Phase through shadow to dodge the next attack.",
          SKILL_EFFECT_DODGE, 0, STATUS_DODGING, 8, 1, 1, 5 },
        { "Twin Strike",  "Two rapid strikes in a single turn.",
          SKILL_EFFECT_MULTI_HIT, 65, STATUS_NONE, 12, 2, 1, 5 },
        { "Smoke Bomb",  "Blind the enemy, reducing their speed for 3 turns.",
          SKILL_EFFECT_DEBUFF, 4, STATUS_SLOWED, 10, 1, 1, 5 },
    };
    memcpy(g_rogue_skills, s, sizeof(s));
}

static void init_mage_skills(void)
{
    SkillDef s[] = {
        { "Fireball",       "Launch a ball of fire for high magic damage.",
          SKILL_EFFECT_DAMAGE, 160, STATUS_NONE, 12, 1, 1, 5 },
        { "Ice Shard",      "A shard of ice that slows the enemy.",
          SKILL_EFFECT_DAMAGE, 110, STATUS_SLOWED, 10, 1, 1, 5 },
        { "Chain Lightning","Unleash a bolt of lightning for massive damage.",
          SKILL_EFFECT_DAMAGE, 200, STATUS_NONE, 20, 1, 1, 5 },
        { "Mana Shield",    "Erect a barrier that absorbs the next attack.",
          SKILL_EFFECT_DODGE, 0, STATUS_SHIELDED, 10, 1, 1, 5 },
        { "Heal",           "Channel life energy to restore your health.",
          SKILL_EFFECT_HEAL, 40, STATUS_NONE, 14, 1, 1, 5 },
    };
    memcpy(g_mage_skills, s, sizeof(s));
}

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
// ─────────────────────────────────────────────
ItemDef g_items[MAX_ITEMS];
int     g_items_count;

static void init_items(void)
{
    ItemDef it[] = {
        { "Nothing",        ITEM_NONE,       0,  0, 0, 0, 0,    0 },
        { "Iron Knife",     ITEM_WEAPON,     0,  0, 2, 1, 0,   50 },
        { "Energy Knife",   ITEM_WEAPON,     0,  5, 3, 2, 0,  100 },
        { "Silver Knife",   ITEM_WEAPON,     0,  0, 4, 3, 0,  160 },
        { "Spiked Axe",     ITEM_WEAPON,     0,  0, 6, 0, 0,  200 },
        { "Heavy Blade",    ITEM_WEAPON,     0,  0, 8, 0, 1,  280 },
        { "Katana",         ITEM_WEAPON,     0,  0, 7, 2, 0,  320 },
        { "Shadow Katana",  ITEM_WEAPON,     0,  8,10, 3, 0,  500 },
        { "Fusion Edge",    ITEM_WEAPON,     0, 10,12, 2, 0,  700 },
        { "Ion Bo",         ITEM_WEAPON,     0, 15, 8, 5, 0,  650 },
        { "Golden Blade",   ITEM_WEAPON,     5,  5,14, 2, 2, 1000 },
        { "Leather Armour", ITEM_ARMOR,     10,  0, 0, 0, 1,   60 },
        { "Metal Plates",   ITEM_ARMOR,     15,  0, 0,-1, 3,  140 },
        { "Metal Armour",   ITEM_ARMOR,     20,  0, 1,-1, 4,  220 },
        { "Snake Skin",     ITEM_ARMOR,     12,  5, 0, 2, 2,  260 },
        { "Shadow Armour",  ITEM_ARMOR,     25, 10, 0, 1, 6,  480 },
        { "Golden Armour",  ITEM_ARMOR,     30,  5, 2, 0, 8,  800 },
        { "Leather Wrist",  ITEM_ACCESSORY,  0,  0, 1, 1, 0,   40 },
        { "Metal Wrist",    ITEM_ACCESSORY,  0,  0, 2, 0, 1,   80 },
        { "Bandana",        ITEM_ACCESSORY,  5,  0, 0, 2, 0,   70 },
        { "Metal Band",     ITEM_ACCESSORY,  5,  0, 1, 1, 1,  130 },
        { "Focus Ring",     ITEM_ACCESSORY,  0, 15, 0, 2, 0,  200 },
        { "Warrior Ring",   ITEM_ACCESSORY,  0,  0, 4, 0, 2,  300 },
        { "Small Potion",   ITEM_CONSUMABLE, 0,  0, 0, 0, 0,   30 },
        { "Large Potion",   ITEM_CONSUMABLE, 0,  0, 0, 0, 0,   80 },
        { "Mana Vial",      ITEM_CONSUMABLE, 0,  0, 0, 0, 0,   40 },
        { "Elixir",         ITEM_CONSUMABLE, 0,  0, 0, 0, 0,  150 },
    };
    g_items_count = (int)(sizeof(it) / sizeof(it[0]));
    memcpy(g_items, it, sizeof(it));
}

// Consumable heal effects (indexed by item id)
static const ConsumeEffect s_consume_defaults[] = {
    [23] = { 30,  0 },
    [24] = { 80,  0 },
    [25] = {  0, 40 },
    [26] = { 60, 40 },
};

ConsumeEffect g_consume_effect[MAX_ITEMS];
const int g_consume_effect_count = MAX_ITEMS;

// ─────────────────────────────────────────────
//  Shop inventories (mutable; may be overridden by shops.dat)
// ─────────────────────────────────────────────
int g_shop_basic[MAX_ITEMS];
int g_shop_basic_count;
int g_shop_mid[MAX_ITEMS];
int g_shop_mid_count;
int g_shop_adv[MAX_ITEMS];
int g_shop_adv_count;

static void init_shops(void)
{
    int basic[] = { 1, 11, 17, 19, 23, 24, 25 };
    int mid[]   = { 3, 4, 5, 13, 14, 18, 20, 21, 24, 25, 26 };
    int adv[]   = { 7, 8, 9, 10, 15, 16, 22, 26 };
    g_shop_basic_count = (int)(sizeof(basic)/sizeof(basic[0]));
    g_shop_mid_count   = (int)(sizeof(mid)/sizeof(mid[0]));
    g_shop_adv_count   = (int)(sizeof(adv)/sizeof(adv[0]));
    memcpy(g_shop_basic, basic, sizeof(basic));
    memcpy(g_shop_mid,   mid,   sizeof(mid));
    memcpy(g_shop_adv,   adv,   sizeof(adv));
}

// ─────────────────────────────────────────────
//  Enemy definitions
// ─────────────────────────────────────────────
const EnemySkill g_enemy_skill_table[] = {
    { SKILL_EFFECT_DAMAGE,  120, "Heavy Blow"    },
    { SKILL_EFFECT_DOT,      15, "Poison Strike" },
    { SKILL_EFFECT_DAMAGE,  180, "Power Slash"   },
    { SKILL_EFFECT_BUFF,      5, "Rage"          },
    { SKILL_EFFECT_DAMAGE,  150, "Fireball"      },
    { SKILL_EFFECT_DEBUFF,    3, "Weaken"        },
    { SKILL_EFFECT_DAMAGE,  200, "Dark Strike"   },
    { SKILL_EFFECT_STUN,     60, "Bash"          },
    { SKILL_EFFECT_DAMAGE,  250, "Shadow Slash"  },
    { SKILL_EFFECT_HEAL,     30, "Mend"          },
};
const int g_enemy_skill_table_count =
    (int)(sizeof(g_enemy_skill_table) / sizeof(g_enemy_skill_table[0]));

EnemyDef g_enemies[MAX_ENEMIES];
int      g_enemies_count;

static void init_enemies(void)
{
    EnemyDef e[] = {
        { "Thief",       { 40,  0, 5, 6,0}, 12, 15, {-1,-1,-1}, (Color){180,120, 60,255} },
        { "Bandit",      { 55,  0, 7, 5,1}, 18, 22, { 0,-1,-1}, (Color){150, 90, 50,255} },
        { "Raider",      { 70,  0, 9, 5,1}, 25, 30, { 0, 7,-1}, (Color){160, 80, 40,255} },
        { "Warrior",     { 90, 20,11, 6,2}, 35, 42, { 0, 3,-1}, (Color){100,100,180,255} },
        { "Seer",        { 60, 60, 8, 7,0}, 30, 38, { 4, 5,-1}, (Color){120, 60,200,255} },
        { "Mercenary",   {100,  0,13, 6,2}, 45, 55, { 2,-1,-1}, (Color){130,130, 70,255} },
        { "Assailant",   { 85, 10,12, 9,1}, 40, 50, { 0, 1,-1}, (Color){ 80,160, 80,255} },
        { "Shaman",      { 75, 80,10, 7,1}, 50, 60, { 4, 9,-1}, (Color){200, 60,200,255} },
        { "Agent",       {110, 20,14, 8,2}, 60, 70, { 2, 5,-1}, (Color){ 60, 60, 60,255} },
        { "Ronin",       {120,  0,15, 9,3}, 70, 85, { 2, 3,-1}, (Color){200,160, 60,255} },
        { "Samurai",     {150, 30,18,10,4}, 90,110, { 2, 3, 7}, (Color){180, 40, 40,255} },
        { "Skeleton",    { 80,  0,10, 5,2}, 30, 35, { 0,-1,-1}, (Color){220,220,200,255} },
        { "Zombie",      {100,  0,12, 3,3}, 38, 42, { 0, 7,-1}, (Color){ 80,160, 80,255} },
        { "Goblin",      { 70, 10, 9, 9,0}, 32, 38, { 1,-1,-1}, (Color){ 60,180, 60,255} },
        { "Troll",       {160,  0,16, 4,4}, 55, 65, { 3,-1,-1}, (Color){ 80,120, 60,255} },
        { "Blood Spirit",{ 90,100,14,10,1}, 70, 80, { 4, 6,-1}, (Color){200, 30, 30,255} },
        { "Shadow Guard",{180, 40,20,10,5},100,120, { 6, 3,-1}, (Color){ 40, 40, 80,255} },
        { "Void Crawler",{160, 60,18,12,4},110,130, { 6, 5,-1}, (Color){ 60, 20, 80,255} },
        { "Dark Knight", {220, 30,24,10,7},130,160, { 8, 3, 7}, (Color){ 20, 20, 50,255} },
        { "Soul Reaper", {200,100,22,13,5},150,180, { 8, 6,-1}, (Color){ 80,  0,120,255} },
        { "Warlord Baka",{300, 60,28,12,8},250,300, { 8, 3, 7}, (Color){180, 10, 10,255} },
    };
    g_enemies_count = (int)(sizeof(e)/sizeof(e[0]));
    memcpy(g_enemies, e, sizeof(e));
}

// ─────────────────────────────────────────────
//  Gateway enemy maps
// ─────────────────────────────────────────────
int g_gw_enemies[NUM_GATEWAYS][GATEWAY_DEPTH];

static void init_gw_enemies(void)
{
    int def[NUM_GATEWAYS][GATEWAY_DEPTH] = {
        { 0, 2, 4, 7, 10 },
        { 11, 12, 13, 14, 15 },
        { 16, 17, 18, 19, 20 },
    };
    memcpy(g_gw_enemies, def, sizeof(def));
}

// ─────────────────────────────────────────────
//  Public init – call once before game starts
// ─────────────────────────────────────────────
void data_init_defaults(void)
{
    init_warrior_skills();
    init_rogue_skills();
    init_mage_skills();
    init_items();
    init_enemies();
    init_gw_enemies();
    init_shops();
    memcpy(g_consume_effect, s_consume_defaults,
           sizeof(s_consume_defaults));
}
