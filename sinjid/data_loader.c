/* data_loader.c – runtime .dat file parser
   Format:
     # comment
     [section_name]       – starts a new record
     key=value            – assigns a field

   One .dat file per data type: enemies.dat, items.dat, skills.dat
*/
#include "data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// ── Helpers ───────────────────────────────────────────────────────────────

static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
    return s;
}

static void kv(const char *line, char *key, int ksz, char *val, int vsz)
{
    const char *eq = strchr(line, '=');
    if (!eq) { key[0] = val[0] = '\0'; return; }
    int klen = (int)(eq - line);
    if (klen >= ksz) klen = ksz - 1;
    strncpy(key, line, klen); key[klen] = '\0';
    strncpy(val, eq + 1, vsz - 1); val[vsz - 1] = '\0';
    // trim
    char *k2 = trim(key); if (k2 != key) memmove(key, k2, strlen(k2)+1);
    char *v2 = trim(val); if (v2 != val) memmove(val, v2, strlen(v2)+1);
}

// ── Effect name → enum ────────────────────────────────────────────────────

static SkillEffectType parse_effect(const char *s)
{
    if (strcmp(s, "damage")   == 0) return SKILL_EFFECT_DAMAGE;
    if (strcmp(s, "heal")     == 0) return SKILL_EFFECT_HEAL;
    if (strcmp(s, "buff")     == 0) return SKILL_EFFECT_BUFF;
    if (strcmp(s, "debuff")   == 0) return SKILL_EFFECT_DEBUFF;
    if (strcmp(s, "dot")      == 0) return SKILL_EFFECT_DOT;
    if (strcmp(s, "stun")     == 0) return SKILL_EFFECT_STUN;
    if (strcmp(s, "dodge")    == 0) return SKILL_EFFECT_DODGE;
    if (strcmp(s, "multihit") == 0) return SKILL_EFFECT_MULTI_HIT;
    return SKILL_EFFECT_DAMAGE;
}

static StatusFlags parse_status(const char *s)
{
    if (strcmp(s, "poisoned") == 0) return STATUS_POISONED;
    if (strcmp(s, "stunned")  == 0) return STATUS_STUNNED;
    if (strcmp(s, "slowed")   == 0) return STATUS_SLOWED;
    if (strcmp(s, "powered")  == 0) return STATUS_POWERED;
    if (strcmp(s, "shielded") == 0) return STATUS_SHIELDED;
    if (strcmp(s, "dodging")  == 0) return STATUS_DODGING;
    return STATUS_NONE;
}

static ItemType parse_item_type(const char *s)
{
    if (strcmp(s, "weapon")     == 0) return ITEM_WEAPON;
    if (strcmp(s, "armor")      == 0) return ITEM_ARMOR;
    if (strcmp(s, "accessory")  == 0) return ITEM_ACCESSORY;
    if (strcmp(s, "consumable") == 0) return ITEM_CONSUMABLE;
    return ITEM_NONE;
}

// ── skills.dat ────────────────────────────────────────────────────────────

static int load_skills(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    SkillDef tmp = {0};
    PlayerClass cur_class = CLASS_WARRIOR;
    int idx = 0;
    bool in_skill = false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *l = trim(line);
        if (!*l || *l == '#') continue;

        if (*l == '[') {
            // save previous record
            if (in_skill && idx < MAX_SKILLS) {
                SkillDef *arr = (cur_class == CLASS_WARRIOR) ? g_warrior_skills :
                                (cur_class == CLASS_ROGUE)   ? g_rogue_skills   :
                                                                g_mage_skills;
                arr[idx++] = tmp;
            }
            in_skill = (strncmp(l+1, "skill", 5) == 0);
            if (in_skill) { memset(&tmp, 0, sizeof(tmp)); tmp.hits = 1; tmp.max_level = 5; tmp.level = 1; }
            continue;
        }

        char key[64], val[128];
        kv(l, key, sizeof(key), val, sizeof(val));

        if (strcmp(key, "class") == 0) {
            if (in_skill && idx > 0) {
                /* flush previous class */
                SkillDef *arr = (cur_class == CLASS_WARRIOR) ? g_warrior_skills :
                                (cur_class == CLASS_ROGUE)   ? g_rogue_skills   :
                                                                g_mage_skills;
                arr[idx-1] = tmp; /* already saved above, but reset idx */
            }
            cur_class = (strcmp(val, "rogue") == 0) ? CLASS_ROGUE :
                        (strcmp(val, "mage")  == 0) ? CLASS_MAGE  : CLASS_WARRIOR;
            idx = 0;
        } else if (strcmp(key, "name")        == 0) strncpy(tmp.name, val, sizeof(tmp.name)-1);
        else if (strcmp(key, "desc")          == 0) strncpy(tmp.desc, val, sizeof(tmp.desc)-1);
        else if (strcmp(key, "effect")        == 0) tmp.effect       = parse_effect(val);
        else if (strcmp(key, "base_value")    == 0) tmp.base_value   = atoi(val);
        else if (strcmp(key, "apply_status")  == 0) tmp.apply_status = parse_status(val);
        else if (strcmp(key, "mana_cost")     == 0) tmp.mana_cost    = atoi(val);
        else if (strcmp(key, "hits")          == 0) tmp.hits         = atoi(val);
        else if (strcmp(key, "max_level")     == 0) tmp.max_level    = atoi(val);
    }
    // flush last
    if (in_skill && idx < MAX_SKILLS) {
        SkillDef *arr = (cur_class == CLASS_WARRIOR) ? g_warrior_skills :
                        (cur_class == CLASS_ROGUE)   ? g_rogue_skills   :
                                                        g_mage_skills;
        arr[idx] = tmp;
    }
    fclose(f);
    return 1;
}

// ── items.dat ─────────────────────────────────────────────────────────────

static int load_items(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    ItemDef tmp = {0};
    int idx = 0;
    bool in_item = false;
    int consume_life = 0, consume_mana = 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *l = trim(line);
        if (!*l || *l == '#') continue;

        if (*l == '[') {
            if (in_item && idx < MAX_ITEMS) {
                g_items[idx] = tmp;
                if (tmp.type == ITEM_CONSUMABLE) {
                    g_consume_effect[idx].life = consume_life;
                    g_consume_effect[idx].mana = consume_mana;
                }
                idx++;
            }
            in_item = (strncmp(l+1, "item", 4) == 0);
            if (in_item) { memset(&tmp, 0, sizeof(tmp)); consume_life = consume_mana = 0; }
            continue;
        }
        if (!in_item) continue;

        char key[64], val[128];
        kv(l, key, sizeof(key), val, sizeof(val));

        if      (strcmp(key, "name")       == 0) strncpy(tmp.name, val, sizeof(tmp.name)-1);
        else if (strcmp(key, "type")       == 0) tmp.type       = parse_item_type(val);
        else if (strcmp(key, "bonus_life") == 0) tmp.bonus_life = atoi(val);
        else if (strcmp(key, "bonus_mana") == 0) tmp.bonus_mana = atoi(val);
        else if (strcmp(key, "bonus_str")  == 0) tmp.bonus_str  = atoi(val);
        else if (strcmp(key, "bonus_spd")  == 0) tmp.bonus_spd  = atoi(val);
        else if (strcmp(key, "bonus_def")  == 0) tmp.bonus_def  = atoi(val);
        else if (strcmp(key, "price")      == 0) tmp.price      = atoi(val);
        else if (strcmp(key, "heal_life")  == 0) consume_life   = atoi(val);
        else if (strcmp(key, "heal_mana")  == 0) consume_mana   = atoi(val);
    }
    if (in_item && idx < MAX_ITEMS) {
        g_items[idx] = tmp;
        if (tmp.type == ITEM_CONSUMABLE) {
            g_consume_effect[idx].life = consume_life;
            g_consume_effect[idx].mana = consume_mana;
        }
        idx++;
    }
    g_items_count = idx;
    fclose(f);
    return 1;
}

// ── enemies.dat ───────────────────────────────────────────────────────────

static int load_enemies(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    EnemyDef tmp = {0};
    int idx = 0;
    bool in_enemy = false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *l = trim(line);
        if (!*l || *l == '#') continue;

        if (*l == '[') {
            if (in_enemy && idx < MAX_ENEMIES) g_enemies[idx++] = tmp;
            in_enemy = (strncmp(l+1, "enemy", 5) == 0);
            if (in_enemy) {
                memset(&tmp, 0, sizeof(tmp));
                tmp.skill_ids[0] = tmp.skill_ids[1] = tmp.skill_ids[2] = -1;
                tmp.color = (Color){180, 180, 180, 255};
            }
            continue;
        }
        if (!in_enemy) continue;

        char key[64], val[128];
        kv(l, key, sizeof(key), val, sizeof(val));

        if      (strcmp(key, "name")     == 0) strncpy(tmp.name, val, sizeof(tmp.name)-1);
        else if (strcmp(key, "max_life") == 0) tmp.base_stats.max_life  = atoi(val);
        else if (strcmp(key, "max_mana") == 0) tmp.base_stats.max_mana  = atoi(val);
        else if (strcmp(key, "strength") == 0) tmp.base_stats.strength  = atoi(val);
        else if (strcmp(key, "speed")    == 0) tmp.base_stats.speed     = atoi(val);
        else if (strcmp(key, "defense")  == 0) tmp.base_stats.defense   = atoi(val);
        else if (strcmp(key, "xp")       == 0) tmp.xp_reward   = atoi(val);
        else if (strcmp(key, "gold")     == 0) tmp.gold_reward = atoi(val);
        else if (strcmp(key, "skill0")   == 0) tmp.skill_ids[0] = atoi(val);
        else if (strcmp(key, "skill1")   == 0) tmp.skill_ids[1] = atoi(val);
        else if (strcmp(key, "skill2")   == 0) tmp.skill_ids[2] = atoi(val);
        else if (strcmp(key, "color")    == 0) {
            int r=0,g=0,b=0;
            sscanf(val, "%d,%d,%d", &r, &g, &b);
            tmp.color = (Color){(unsigned char)r,(unsigned char)g,(unsigned char)b,255};
        }
    }
    if (in_enemy && idx < MAX_ENEMIES) g_enemies[idx++] = tmp;
    g_enemies_count = idx;
    fclose(f);
    return 1;
}

// ── gateways.dat ──────────────────────────────────────────────────────────

static int load_gateways(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int gw = -1, floor_idx = 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char *l = trim(line);
        if (!*l || *l == '#') continue;

        if (*l == '[') {
            floor_idx = 0;
            if      (strstr(l, "human")   != NULL) gw = GW_HUMAN;
            else if (strstr(l, "monster") != NULL) gw = GW_MONSTER;
            else if (strstr(l, "dark")    != NULL) gw = GW_DARK_RIFT;
            else gw = -1;
            continue;
        }
        if (gw < 0 || floor_idx >= GATEWAY_DEPTH) continue;

        char key[32], val[32];
        kv(l, key, sizeof(key), val, sizeof(val));
        if (strncmp(key, "floor", 5) == 0)
            g_gw_enemies[gw][floor_idx++] = atoi(val);
    }
    fclose(f);
    return 1;
}

// ── shops.dat ─────────────────────────────────────────────────────────────

static int load_shops(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int  *cur_arr   = NULL;
    int  *cur_count = NULL;
    int   idx       = 0;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char *l = trim(line);
        if (!*l || *l == '#') continue;

        if (*l == '[') {
            if (cur_arr && cur_count) *cur_count = idx;
            if      (strstr(l, "basic")   != NULL) { cur_arr = g_shop_basic; cur_count = &g_shop_basic_count; }
            else if (strstr(l, "mid")     != NULL) { cur_arr = g_shop_mid;   cur_count = &g_shop_mid_count;   }
            else if (strstr(l, "adv")     != NULL) { cur_arr = g_shop_adv;   cur_count = &g_shop_adv_count;   }
            else                                   { cur_arr = NULL; cur_count = NULL; }
            idx = 0;
            continue;
        }
        if (!cur_arr) continue;

        char key[32], val[32];
        kv(l, key, sizeof(key), val, sizeof(val));
        if (strcmp(key, "item") == 0 && idx < MAX_ITEMS)
            cur_arr[idx++] = atoi(val);
    }
    if (cur_arr && cur_count) *cur_count = idx;

    fclose(f);
    return 1;
}

// ── Public entry point ────────────────────────────────────────────────────

int data_load_files(const char *dir)
{
    char path[256];
    int loaded = 0;

    snprintf(path, sizeof(path), "%s/skills.dat",   dir); loaded += load_skills(path);
    snprintf(path, sizeof(path), "%s/items.dat",    dir); loaded += load_items(path);
    snprintf(path, sizeof(path), "%s/enemies.dat",  dir); loaded += load_enemies(path);
    snprintf(path, sizeof(path), "%s/gateways.dat", dir); loaded += load_gateways(path);
    snprintf(path, sizeof(path), "%s/shops.dat",    dir); loaded += load_shops(path);

    return loaded;
}
