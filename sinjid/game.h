#pragma once
#include <raylib.h>
#include <stdbool.h>
#include "scene.h"
#include "events.h"

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
#define MAX_ITEMS        48
#define MAX_COMBAT_LOG   8
#define COMBAT_LOG_LEN   128
#define GATEWAY_DEPTH    5
#define NUM_GATEWAYS     3

// ─────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────
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
    STATUS_POISONED  = (1 << 0),
    STATUS_STUNNED   = (1 << 1),
    STATUS_SLOWED    = (1 << 2),
    STATUS_POWERED   = (1 << 3),
    STATUS_SHIELDED  = (1 << 4),
    STATUS_DODGING   = (1 << 5),
} StatusFlags;

typedef enum {
    SKILL_EFFECT_DAMAGE,
    SKILL_EFFECT_HEAL,
    SKILL_EFFECT_BUFF,
    SKILL_EFFECT_DEBUFF,
    SKILL_EFFECT_DOT,
    SKILL_EFFECT_STUN,
    SKILL_EFFECT_DODGE,
    SKILL_EFFECT_MULTI_HIT,
} SkillEffectType;

// ─────────────────────────────────────────────
//  Data structures
// ─────────────────────────────────────────────
typedef struct {
    int max_life, max_mana, strength, speed, defense;
} Stats;

typedef struct {
    char        name[32];
    char        desc[128];
    SkillEffectType effect;
    int base_value;
    StatusFlags apply_status;
    int mana_cost;
    int hits;
    int level;
    int max_level;
} SkillDef;

typedef struct {
    char     name[64];
    ItemType type;
    int      bonus_life, bonus_mana, bonus_str, bonus_spd, bonus_def;
    int      price;
} ItemDef;

typedef struct {
    char        name[64];
    Stats       base_stats;
    int         xp_reward, gold_reward;
    int         skill_ids[3];
    Color       color;
    StatusFlags status;
    int         status_turns;
    int         current_life, current_mana;
} EnemyDef;

typedef struct {
    char        name[32];
    PlayerClass pc;
    int         level, xp, xp_to_next, gold;
    int         stat_points, skill_points;
    Stats       base;
    int         current_life, current_mana;
    int         equip[EQUIP_SLOT_COUNT];
    int         bag_ids[MAX_INVENTORY];
    int         bag_qty[MAX_INVENTORY];
    int         skill_level[MAX_SKILLS];
    StatusFlags status;
    int         status_turns;
    int         gw_progress[NUM_GATEWAYS];
    bool        gw_complete[NUM_GATEWAYS];
} Player;

typedef struct {
    EnemyDef  enemy;
    bool      player_turn, combat_over, player_won, fled;
    GatewayId gateway;
    int       floor;
    int       str_bonus, spd_bonus, def_bonus;
    char      log[MAX_COMBAT_LOG][COMBAT_LOG_LEN];
    int       log_count;
    bool      show_skills, show_items;
} CombatState;

typedef struct {
    const int *item_ids;
    int        count, selected;
} ShopState;

typedef struct {
    int levels_gained, stat_pts, skill_pts;
} LevelUpState;

// ── Animation state ──────────────────────────────────────────────────────
typedef struct {
    float player_life, player_mana;
    float enemy_life,  enemy_mana;
    float player_flash, enemy_flash;
    bool  player_flash_col, enemy_flash_col;
    float player_bob_t, enemy_bob_t;
} CombatAnim;

typedef struct {
    float fade_alpha;
    bool  fading_out, active;
    int   target_scene;
} FadeAnim;

// ─────────────────────────────────────────────
//  Central game context
// ─────────────────────────────────────────────
typedef struct {
    SceneStack   scenes;
    Player       player;
    CombatState  combat;
    ShopState    shop;
    LevelUpState lvlup;
    CombatAnim   canim;
    FadeAnim     fade;
    EventQueue   events;
    bool         scene_dirty;
} GameCtx;

// Single global instance – defined in main.c
extern GameCtx g_ctx;

// Convenience accessors (read-only alias macros)
#define G_SCENE       scene_current(&g_ctx.scenes)
#define G_PLAYER      g_ctx.player
#define G_COMBAT      g_ctx.combat
#define G_SHOP        g_ctx.shop
#define G_CANIM       g_ctx.canim
#define G_FADE        g_ctx.fade
#define G_DIRTY       g_ctx.scene_dirty
#define G_EVENTS      g_ctx.events

// ─────────────────────────────────────────────
//  Data tables (populated by data_load())
// ─────────────────────────────────────────────
extern SkillDef g_warrior_skills[MAX_SKILLS];
extern SkillDef g_rogue_skills[MAX_SKILLS];
extern SkillDef g_mage_skills[MAX_SKILLS];
extern ItemDef  g_items[MAX_ITEMS];
extern int      g_items_count;
extern EnemyDef g_enemies[MAX_ENEMIES];
extern int      g_enemies_count;
extern int      g_gw_enemies[NUM_GATEWAYS][GATEWAY_DEPTH];

extern const int g_shop_basic[];
extern const int g_shop_basic_count;
extern const int g_shop_mid[];
extern const int g_shop_mid_count;
extern const int g_shop_adv[];
extern const int g_shop_adv_count;

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
#define CLAMP(v,lo,hi) ((v)<(lo)?(lo):(v)>(hi)?(hi):(v))
#define MAX(a,b)       ((a)>(b)?(a):(b))
#define MIN(a,b)       ((a)<(b)?(a):(b))

int  player_effective_str(void);
int  player_effective_spd(void);
int  player_effective_def(void);
int  player_effective_life(void);
int  player_effective_mana(void);
bool gateway_unlocked(GatewayId gw);
