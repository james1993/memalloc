#pragma once
#include <raylib.h>
#include <stdbool.h>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
#define SCREEN_W         1024
#define SCREEN_H         768
#define TARGET_FPS       60
#define GAME_TITLE       "Sinjid: Shadow of the Warrior"

#define MAX_INVENTORY    24
#define MAX_SKILLS       5
#define MAX_ENEMIES      40
#define MAX_ITEMS        32
#define MAX_COMBAT_LOG   8
#define COMBAT_LOG_LEN   128
#define GATEWAY_DEPTH    5   // enemies per gateway level
#define NUM_GATEWAYS     3

// ─────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────
typedef enum {
    SCENE_TITLE,
    SCENE_CLASS_SELECT,
    SCENE_HUB,
    SCENE_COMBAT,
    SCENE_INVENTORY,
    SCENE_SHOP,
    SCENE_SKILLS,
    SCENE_LEVEL_UP,
    SCENE_GATEWAY_SELECT,
    SCENE_GAME_OVER,
    SCENE_VICTORY
} Scene;

typedef enum {
    CLASS_WARRIOR,
    CLASS_ROGUE,
    CLASS_MAGE,
    CLASS_COUNT
} PlayerClass;

typedef enum {
    ITEM_NONE = 0,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_ACCESSORY,
    ITEM_CONSUMABLE
} ItemType;

typedef enum {
    EQUIP_WEAPON,
    EQUIP_ARMOR,
    EQUIP_ACCESSORY,
    EQUIP_SLOT_COUNT
} EquipSlot;

typedef enum {
    GW_HUMAN,
    GW_MONSTER,
    GW_DARK_RIFT
} GatewayId;

typedef enum {
    STATUS_NONE      = 0,
    STATUS_POISONED  = (1 << 0),   // 3 dmg/turn
    STATUS_STUNNED   = (1 << 1),   // skip turn
    STATUS_SLOWED    = (1 << 2),   // -3 speed
    STATUS_POWERED   = (1 << 3),   // +4 strength
    STATUS_SHIELDED  = (1 << 4),   // reduce incoming dmg by 30%
    STATUS_DODGING   = (1 << 5),   // dodge next attack
} StatusFlags;

typedef enum {
    SKILL_EFFECT_DAMAGE,
    SKILL_EFFECT_HEAL,
    SKILL_EFFECT_BUFF,
    SKILL_EFFECT_DEBUFF,
    SKILL_EFFECT_DOT,
    SKILL_EFFECT_STUN,
    SKILL_EFFECT_DODGE,
    SKILL_EFFECT_MULTI_HIT,   // hits N times
} SkillEffectType;

// ─────────────────────────────────────────────
//  Data structures
// ─────────────────────────────────────────────

typedef struct {
    int max_life;
    int max_mana;
    int strength;
    int speed;
    int defense;   // flat damage reduction
} Stats;

typedef struct {
    const char *name;
    const char *desc;
    SkillEffectType effect;
    int base_value;           // damage / heal / stat delta
    StatusFlags apply_status; // status to apply on hit (0 = none)
    int mana_cost;
    int hits;                 // for MULTI_HIT; 1 otherwise
    int level;                // 1..5 (upgrades)
    int max_level;
} SkillDef;

typedef struct {
    const char *name;
    ItemType    type;
    // Stat bonuses when equipped
    int bonus_life;
    int bonus_mana;
    int bonus_str;
    int bonus_spd;
    int bonus_def;
    int price;               // buy price; sell = price/2
} ItemDef;

typedef struct {
    const char *name;
    Stats       base_stats;
    int         xp_reward;
    int         gold_reward;
    // Simple AI: skill indices into a small skill list (up to 3)
    // enemy_skills[i] = index into g_enemy_skills[], -1 = end
    int         skill_ids[3];
    Color       color;        // visual color for sprite rect
    StatusFlags status;
    int         status_turns;
    int         current_life;
    int         current_mana;
} EnemyDef;

// Runtime player
typedef struct {
    char        name[32];
    PlayerClass pc;
    int         level;
    int         xp;
    int         xp_to_next;
    int         gold;
    int         stat_points;    // unspent
    int         skill_points;   // unspent

    Stats       base;           // permanent stat pool
    int         current_life;
    int         current_mana;

    // Equipment slots: index into g_items[], -1 = empty
    int         equip[EQUIP_SLOT_COUNT];
    // Bag: item id + quantity
    int         bag_ids[MAX_INVENTORY];
    int         bag_qty[MAX_INVENTORY];

    // Skill levels (per-class, indexed 0..MAX_SKILLS-1)
    int         skill_level[MAX_SKILLS];

    StatusFlags status;
    int         status_turns;

    // Gateway progress: current floor index (0 = not started)
    int         gw_progress[NUM_GATEWAYS];
    bool        gw_complete[NUM_GATEWAYS];
} Player;

// Combat state (transient)
typedef struct {
    EnemyDef    enemy;            // copy of the enemy being fought
    bool        player_turn;
    bool        combat_over;
    bool        player_won;
    bool        fled;
    // Which gateway / floor triggered this combat
    GatewayId   gateway;
    int         floor;
    // Pending stat changes from buffs (reset each combat)
    int         str_bonus;
    int         spd_bonus;
    int         def_bonus;
    // Log
    char        log[MAX_COMBAT_LOG][COMBAT_LOG_LEN];
    int         log_count;
    // Submenu state
    bool        show_skills;
    bool        show_items;
    bool        animating;
    float       anim_timer;
} CombatState;

// Shop state (transient)
typedef struct {
    const int  *item_ids;
    int         count;
    int         selected;
} ShopState;

// Level-up state
typedef struct {
    int levels_gained;
    int stat_pts;
    int skill_pts;
} LevelUpState;

// ─────────────────────────────────────────────
//  Globals (defined in main.c)
// ─────────────────────────────────────────────
extern Scene        g_scene;
extern Player       g_player;
extern CombatState  g_combat;
extern ShopState    g_shop;
extern LevelUpState g_lvlup;
extern int          g_hover_btn;   // index of hovered button this frame

// ─────────────────────────────────────────────
//  Forward declarations of subsystems
// ─────────────────────────────────────────────
// data.h
extern const SkillDef g_warrior_skills[MAX_SKILLS];
extern const SkillDef g_rogue_skills[MAX_SKILLS];
extern const SkillDef g_mage_skills[MAX_SKILLS];
extern const ItemDef  g_items[];
extern const int      g_items_count;
extern const EnemyDef g_enemies[];

// Gateway enemy lists: g_gw_enemies[gateway][floor] = index into g_enemies[]
extern const int g_gw_enemies[NUM_GATEWAYS][GATEWAY_DEPTH];

// Beginner shop & mid shop item id lists
extern const int g_shop_basic[];
extern const int g_shop_basic_count;
extern const int g_shop_mid[];
extern const int g_shop_mid_count;

// ─────────────────────────────────────────────
//  Helper macros
// ─────────────────────────────────────────────
#define CLAMP(v, lo, hi)  ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))
#define MAX(a, b)         ((a) > (b) ? (a) : (b))
#define MIN(a, b)         ((a) < (b) ? (a) : (b))

// Effective stat = base + equipment bonus + combat buff
int player_effective_str(void);
int player_effective_spd(void);
int player_effective_def(void);
int player_effective_life(void);
int player_effective_mana(void);

// Check if player has unlocked a gateway
bool gateway_unlocked(GatewayId gw);
