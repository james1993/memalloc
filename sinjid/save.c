/* save.c – field-by-field text serialization
 *
 * Format: plain text, one "key=value" per line.
 * Unknown keys are silently ignored (forward-compatible).
 * A "version" key guards against incompatible future changes.
 *
 * This replaces the old raw-struct binary dump so that:
 *   - Adding new Player fields doesn't corrupt old saves.
 *   - Save files are human-readable / hand-editable.
 */
#include "save.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define SPATH g_config.save_path

/* ── Helpers ── */
static char *ltrim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    return s;
}

static void write_int(FILE *f, const char *key, int v)
{
    fprintf(f, "%s=%d\n", key, v);
}
static void write_str(FILE *f, const char *key, const char *v)
{
    fprintf(f, "%s=%s\n", key, v);
}

/* ── Save ──────────────────────────────────────────────────────────────── */
bool save_game(const Player *p)
{
    FILE *f = fopen(SPATH, "w");
    if (!f) return false;

    write_int(f, "version",       SAVE_VERSION);
    write_str(f, "name",          p->name);
    write_int(f, "class",         (int)p->pc);
    write_int(f, "level",         p->level);
    write_int(f, "xp",            p->xp);
    write_int(f, "xp_to_next",    p->xp_to_next);
    write_int(f, "gold",          p->gold);
    write_int(f, "stat_points",   p->stat_points);
    write_int(f, "skill_points",  p->skill_points);
    write_int(f, "current_life",  p->current_life);
    write_int(f, "current_mana",  p->current_mana);
    write_int(f, "base_max_life", p->base.max_life);
    write_int(f, "base_max_mana", p->base.max_mana);
    write_int(f, "base_str",      p->base.strength);
    write_int(f, "base_spd",      p->base.speed);
    write_int(f, "base_def",      p->base.defense);
    write_int(f, "status",        (int)p->status);
    write_int(f, "status_turns",  p->status_turns);

    for (int i = 0; i < EQUIP_SLOT_COUNT; i++) {
        char key[32];
        snprintf(key, sizeof(key), "equip%d", i);
        write_int(f, key, p->equip[i]);
    }
    for (int i = 0; i < MAX_INVENTORY; i++) {
        char key[32];
        snprintf(key, sizeof(key), "bag_id%d", i);
        write_int(f, key, p->bag_ids[i]);
        snprintf(key, sizeof(key), "bag_qty%d", i);
        write_int(f, key, p->bag_qty[i]);
    }
    for (int i = 0; i < MAX_SKILLS; i++) {
        char key[32];
        snprintf(key, sizeof(key), "skill_lv%d", i);
        write_int(f, key, p->skill_level[i]);
    }
    for (int i = 0; i < NUM_GATEWAYS; i++) {
        char key[32];
        snprintf(key, sizeof(key), "gw_prog%d", i);
        write_int(f, key, p->gw_progress[i]);
        snprintf(key, sizeof(key), "gw_done%d", i);
        write_int(f, key, p->gw_complete[i] ? 1 : 0);
    }

    fclose(f);
    return true;
}

/* ── Load ──────────────────────────────────────────────────────────────── */
bool load_game(Player *p)
{
    FILE *f = fopen(SPATH, "r");
    if (!f) return false;

    /* Pre-scan: read only the version field to reject incompatible saves
       before populating any player data. */
    {
        int file_version = 0;
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char *l = ltrim(line);
            if (strncmp(l, "version=", 8) == 0) {
                file_version = atoi(l + 8);
                break;
            }
        }
        if (file_version != SAVE_VERSION) {
            fclose(f);
            return false;
        }
        rewind(f);
    }

    /* Zero-init then set defaults that a partial save shouldn't leave broken */
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < EQUIP_SLOT_COUNT; i++) p->equip[i] = -1;
    for (int i = 0; i < MAX_INVENTORY;    i++) p->bag_ids[i] = -1;
    for (int i = 0; i < MAX_SKILLS;       i++) p->skill_level[i] = 1;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *l = ltrim(line);
        if (!*l || *l == '#') continue;

        char *eq = strchr(l, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = l;
        char *val = eq + 1;
        /* strip trailing newline from val */
        char *nl = strchr(val, '\n'); if (nl) *nl = '\0';
        nl = strchr(val, '\r');       if (nl) *nl = '\0';

        int v = atoi(val);

        if      (strcmp(key, "version")       == 0) { /* already validated */ }
        else if (strcmp(key, "name")          == 0) strncpy(p->name, val, sizeof(p->name)-1);
        else if (strcmp(key, "class")         == 0) p->pc             = (PlayerClass)v;
        else if (strcmp(key, "level")         == 0) p->level          = v;
        else if (strcmp(key, "xp")            == 0) p->xp             = v;
        else if (strcmp(key, "xp_to_next")    == 0) p->xp_to_next     = v;
        else if (strcmp(key, "gold")          == 0) p->gold           = v;
        else if (strcmp(key, "stat_points")   == 0) p->stat_points    = v;
        else if (strcmp(key, "skill_points")  == 0) p->skill_points   = v;
        else if (strcmp(key, "current_life")  == 0) p->current_life   = v;
        else if (strcmp(key, "current_mana")  == 0) p->current_mana   = v;
        else if (strcmp(key, "base_max_life") == 0) p->base.max_life  = v;
        else if (strcmp(key, "base_max_mana") == 0) p->base.max_mana  = v;
        else if (strcmp(key, "base_str")      == 0) p->base.strength  = v;
        else if (strcmp(key, "base_spd")      == 0) p->base.speed     = v;
        else if (strcmp(key, "base_def")      == 0) p->base.defense   = v;
        else if (strcmp(key, "status")        == 0) p->status         = (StatusFlags)v;
        else if (strcmp(key, "status_turns")  == 0) p->status_turns   = v;
        else if (strncmp(key, "equip", 5)     == 0) {
            int i = atoi(key + 5);
            if (i >= 0 && i < EQUIP_SLOT_COUNT) p->equip[i] = v;
        }
        else if (strncmp(key, "bag_id", 6)    == 0) {
            int i = atoi(key + 6);
            if (i >= 0 && i < MAX_INVENTORY) p->bag_ids[i] = v;
        }
        else if (strncmp(key, "bag_qty", 7)   == 0) {
            int i = atoi(key + 7);
            if (i >= 0 && i < MAX_INVENTORY) p->bag_qty[i] = v;
        }
        else if (strncmp(key, "skill_lv", 8)  == 0) {
            int i = atoi(key + 8);
            if (i >= 0 && i < MAX_SKILLS) p->skill_level[i] = v;
        }
        else if (strncmp(key, "gw_prog", 7)   == 0) {
            int i = atoi(key + 7);
            if (i >= 0 && i < NUM_GATEWAYS) p->gw_progress[i] = v;
        }
        else if (strncmp(key, "gw_done", 7)   == 0) {
            int i = atoi(key + 7);
            if (i >= 0 && i < NUM_GATEWAYS) p->gw_complete[i] = (v != 0);
        }
    }

    fclose(f);

    /* Basic sanity */
    if (p->level < 1 || p->level > MAX_PLAYER_LEVEL) return false;

    return true;
}

/* ── Misc ── */
bool save_exists(void)
{
    FILE *f = fopen(SPATH, "r");
    if (!f) return false;
    fclose(f);
    return true;
}

void save_delete(void)
{
    remove(SPATH);
}
