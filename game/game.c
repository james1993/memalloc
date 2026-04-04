// ============================================================
// Vampire Survivors Clone - written in C with Raylib
// Controls: WASD to move, weapons fire automatically
// Survive 15 minutes to win. Level up to choose upgrades.
// ============================================================
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================
// Constants
// ============================================================
#define SCREEN_W        1280
#define SCREEN_H        720
#define TARGET_FPS      60
#define WIN_TIME        900.0f   // 15 minutes
#define SPAWN_RADIUS    750.0f
#define MAX_ENEMIES     600
#define MAX_BULLETS     400
#define MAX_GEMS        500
#define MAX_WEAPONS     8

// ============================================================
// Types
// ============================================================

typedef enum {
    WPN_WAND = 0,   // fires homing bullet at nearest enemy
    WPN_GARLIC,     // area damage aura around player
    WPN_AXE,        // arcing projectile, pierces all
    WPN_CROSS,      // fires in 4 directions
    WPN_ORB,        // orbiting damage orbs
    WPN_COUNT,
} WeaponID;

typedef struct {
    WeaponID id;
    int      level;
    float    cooldown;
    float    timer;
    float    angle;     // used by ORB
} WeaponSlot;

typedef struct {
    Vector2 pos;
    float   hp, maxHp;
    float   speed;
    float   xp, xpNext;
    int     level;
    float   iframes;    // invincibility timer after hit
    int     alive;
} Player;

typedef enum {
    ENEMY_BAT = 0,
    ENEMY_SKELETON,
    ENEMY_ZOMBIE,
    ENEMY_GHOST,
    ENEMY_TYPE_COUNT,
} EnemyType;

typedef struct {
    Vector2   pos;
    EnemyType type;
    float     hp, maxHp;
    float     speed;
    float     damage;
    float     attackCooldown;
    float     radius;
    float     xpDrop;
    int       alive;
    Color     color;
} Enemy;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float   damage;
    float   radius;
    float   lifetime;
    int     pierce;
    int     alive;
    Color   color;
    WeaponID source;
} Bullet;

typedef struct {
    Vector2 pos;
    float   value;
    int     alive;
} Gem;

typedef struct {
    char label[64];
    char desc[128];
    int  type;          // 0=stat, 1=upgrade weapon, 2=new weapon
    int  statIdx;       // type==0: 0=maxhp, 1=speed, 2=recover
    int  weaponSlot;    // type==1
    WeaponID newWpn;    // type==2
} UpgradeOption;

typedef enum {
    STATE_PLAYING,
    STATE_LEVELUP,
    STATE_DEAD,
    STATE_WIN,
} GameState;

// ============================================================
// Globals
// ============================================================
static Player     g_player;
static Enemy      g_enemies[MAX_ENEMIES];
static Bullet     g_bullets[MAX_BULLETS];
static Gem        g_gems[MAX_GEMS];
static WeaponSlot g_weapons[MAX_WEAPONS];
static int        g_weaponCount;

static GameState  g_state;
static float      g_time;
static int        g_kills;
static float      g_spawnTimer;
static float      g_spawnInterval;

static UpgradeOption g_opts[3];
static int           g_pendingLevels;

static Camera2D g_cam;

// ============================================================
// Helpers
// ============================================================
static float RandF(void)             { return (float)rand() / (float)RAND_MAX; }
static float V2Len(Vector2 v)        { return sqrtf(v.x*v.x + v.y*v.y); }
static float V2Dist(Vector2 a, Vector2 b) { float dx=a.x-b.x, dy=a.y-b.y; return sqrtf(dx*dx+dy*dy); }
static Vector2 V2Norm(Vector2 v)     { float l=V2Len(v); if(l<1e-6f)return(Vector2){0,0}; return(Vector2){v.x/l,v.y/l}; }

static Vector2 RandDir(void) {
    float a = RandF() * 2.0f * PI;
    return (Vector2){ cosf(a), sinf(a) };
}

static int FreeEnemy(void)  { for(int i=0;i<MAX_ENEMIES;i++) if(!g_enemies[i].alive) return i; return -1; }
static int FreeBullet(void) { for(int i=0;i<MAX_BULLETS;i++) if(!g_bullets[i].alive) return i; return -1; }
static int FreeGem(void)    { for(int i=0;i<MAX_GEMS;i++)    if(!g_gems[i].alive)    return i; return -1; }

static bool HasWeapon(WeaponID id) {
    for (int i = 0; i < g_weaponCount; i++)
        if (g_weapons[i].id == id) return true;
    return false;
}

static const char *WpnName(WeaponID id) {
    switch (id) {
        case WPN_WAND:   return "Magic Wand";
        case WPN_GARLIC: return "Garlic";
        case WPN_AXE:    return "Axe";
        case WPN_CROSS:  return "Cross";
        case WPN_ORB:    return "Energy Orb";
        default:         return "Unknown";
    }
}

static float WpnCooldown(WeaponID id, int lv) {
    float base[] = { 1.2f, 0.0f, 1.8f, 2.5f, 0.0f }; // garlic/orb: continuous
    float cd = base[id] * (1.0f - (lv - 1) * 0.08f);
    return fmaxf(cd, 0.2f);
}

static float WpnDamage(WeaponID id, int lv) {
    float base[] = { 15.0f, 25.0f, 50.0f, 20.0f, 35.0f };
    return base[id] * (1.0f + (lv - 1) * 0.25f);
}

// ============================================================
// Enemy initialization
// ============================================================
static void InitEnemy(Enemy *e, EnemyType type, Vector2 pos) {
    *e = (Enemy){0};
    e->type  = type;
    e->pos   = pos;
    e->alive = 1;
    switch (type) {
        case ENEMY_BAT:
            e->maxHp=20; e->speed=120; e->damage=8; e->radius=12; e->xpDrop=2; e->color=PURPLE; break;
        case ENEMY_SKELETON:
            e->maxHp=55; e->speed=65;  e->damage=15; e->radius=16; e->xpDrop=5; e->color=LIGHTGRAY; break;
        case ENEMY_ZOMBIE:
            e->maxHp=140; e->speed=42; e->damage=28; e->radius=22; e->xpDrop=10; e->color=GREEN; break;
        case ENEMY_GHOST:
            e->maxHp=35; e->speed=95;  e->damage=12; e->radius=14; e->xpDrop=4; e->color=SKYBLUE; break;
        default: break;
    }
    e->hp = e->maxHp;
}

// ============================================================
// Weapon fire helpers
// ============================================================
static Enemy *NearestEnemy(void) {
    Enemy *best = NULL;
    float minD = 1e9f;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!g_enemies[i].alive) continue;
        float d = V2Dist(g_player.pos, g_enemies[i].pos);
        if (d < minD) { minD = d; best = &g_enemies[i]; }
    }
    return best;
}

static void SpawnBullet(Vector2 vel, float dmg, float radius, float lifetime,
                        int pierce, Color col, WeaponID src) {
    int idx = FreeBullet();
    if (idx < 0) return;
    Bullet *b = &g_bullets[idx];
    b->pos      = g_player.pos;
    b->vel      = vel;
    b->damage   = dmg;
    b->radius   = radius;
    b->lifetime = lifetime;
    b->pierce   = pierce;
    b->color    = col;
    b->source   = src;
    b->alive    = 1;
}

static void FireWand(WeaponSlot *ws) {
    Enemy *t = NearestEnemy();
    Vector2 dir = t
        ? V2Norm((Vector2){t->pos.x - g_player.pos.x, t->pos.y - g_player.pos.y})
        : RandDir();
    float spd = 420.0f;
    SpawnBullet((Vector2){dir.x*spd, dir.y*spd},
        WpnDamage(WPN_WAND, ws->level), 8, 3.0f,
        ws->level / 3, YELLOW, WPN_WAND);
}

static void FireAxe(WeaponSlot *ws) {
    Enemy *t = NearestEnemy();
    int count = 1 + ws->level / 3;
    float baseAngle = t
        ? atan2f(t->pos.y - g_player.pos.y, t->pos.x - g_player.pos.x)
        : RandF() * 2 * PI;
    for (int c = 0; c < count; c++) {
        float a = baseAngle + (c - count / 2) * 0.35f;
        float spd = 520.0f;
        SpawnBullet((Vector2){cosf(a)*spd, sinf(a)*spd},
            WpnDamage(WPN_AXE, ws->level), 14, 1.2f,
            999, ORANGE, WPN_AXE);
    }
}

static void FireCross(WeaponSlot *ws) {
    float spd = 360.0f;
    float dur = 0.8f + ws->level * 0.1f;
    float angles[4] = { 0, PI*0.5f, PI, PI*1.5f };
    for (int i = 0; i < 4; i++) {
        SpawnBullet(
            (Vector2){ cosf(angles[i])*spd, sinf(angles[i])*spd },
            WpnDamage(WPN_CROSS, ws->level), 10, dur,
            ws->level, WHITE, WPN_CROSS);
    }
}

// ============================================================
// Continuous weapons (garlic, orb)
// ============================================================
static void UpdateGarlic(WeaponSlot *ws, float dt) {
    float range = 85.0f + ws->level * 15.0f;
    float dmg   = WpnDamage(WPN_GARLIC, ws->level) * dt;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!g_enemies[i].alive) continue;
        if (V2Dist(g_player.pos, g_enemies[i].pos) < range + g_enemies[i].radius)
            g_enemies[i].hp -= dmg;
    }
}

static void UpdateOrb(WeaponSlot *ws, float dt) {
    ws->angle += dt * 2.8f;
    int   count = 1 + ws->level / 2;
    float range = 80.0f + ws->level * 12.0f;
    float dmg   = WpnDamage(WPN_ORB, ws->level) * dt * 4.0f;
    for (int c = 0; c < count; c++) {
        float a = ws->angle + (float)c * (2.0f * PI / count);
        Vector2 orbPos = {
            g_player.pos.x + cosf(a) * range,
            g_player.pos.y + sinf(a) * range,
        };
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!g_enemies[i].alive) continue;
            if (V2Dist(orbPos, g_enemies[i].pos) < 18.0f + g_enemies[i].radius)
                g_enemies[i].hp -= dmg;
        }
    }
}

// ============================================================
// Enemy spawning
// ============================================================
static void SpawnEnemy(void) {
    int idx = FreeEnemy();
    if (idx < 0) return;
    Vector2 dir = RandDir();
    Vector2 pos = {
        g_player.pos.x + dir.x * SPAWN_RADIUS,
        g_player.pos.y + dir.y * SPAWN_RADIUS,
    };
    EnemyType type;
    float roll = RandF();
    if      (g_time < 60)  { type = ENEMY_BAT; }
    else if (g_time < 120) { type = roll < 0.55f ? ENEMY_BAT : ENEMY_SKELETON; }
    else if (g_time < 240) {
        if      (roll < 0.4f)  type = ENEMY_BAT;
        else if (roll < 0.7f)  type = ENEMY_SKELETON;
        else                   type = ENEMY_GHOST;
    } else {
        if      (roll < 0.25f) type = ENEMY_BAT;
        else if (roll < 0.5f)  type = ENEMY_SKELETON;
        else if (roll < 0.75f) type = ENEMY_GHOST;
        else                   type = ENEMY_ZOMBIE;
    }
    InitEnemy(&g_enemies[idx], type, pos);
    // Scale stats with time
    float scale = 1.0f + g_time / 300.0f;
    g_enemies[idx].maxHp  *= scale;
    g_enemies[idx].hp      = g_enemies[idx].maxHp;
    g_enemies[idx].damage *= 1.0f + g_time / 600.0f;
}

// ============================================================
// Level-up system
// ============================================================
static void BuildUpgradeOptions(void) {
    static const char *statLabels[] = { "Max HP +20%", "Speed +10%", "Recover 30 HP" };
    static const char *statDescs[]  = {
        "Increases your maximum health by 20%.",
        "Move faster across the battlefield.",
        "Instantly restore 30 HP.",
    };

    int filled = 0;
    bool used[3] = {false, false, false}; // which stat indices used

    // 1. Always offer one stat upgrade
    int si = rand() % 3;
    g_opts[filled].type    = 0;
    g_opts[filled].statIdx = si;
    strncpy(g_opts[filled].label, statLabels[si], 63);
    strncpy(g_opts[filled].desc,  statDescs[si],  127);
    used[si] = true;
    filled++;

    // 2. Try to offer a weapon upgrade or new weapon for the remaining slots
    int attempts = 0;
    while (filled < 3 && attempts++ < 80) {
        // Flip a coin: new weapon vs upgrade existing
        bool tryNew = (RandF() < 0.4f) && (g_weaponCount < MAX_WEAPONS) && (g_weaponCount < WPN_COUNT);
        if (tryNew) {
            // Pick a weapon we don't have
            WeaponID cands[WPN_COUNT];
            int nc = 0;
            for (int w = 0; w < WPN_COUNT; w++)
                if (!HasWeapon((WeaponID)w)) cands[nc++] = (WeaponID)w;
            if (nc == 0) { tryNew = false; goto try_upgrade; }

            WeaponID pick = cands[rand() % nc];
            // Check not already offered
            bool dup = false;
            for (int j = 0; j < filled; j++)
                if (g_opts[j].type == 2 && g_opts[j].newWpn == pick) { dup = true; break; }
            if (dup) continue;

            g_opts[filled].type   = 2;
            g_opts[filled].newWpn = pick;
            snprintf(g_opts[filled].label, 64,  "New: %s", WpnName(pick));
            snprintf(g_opts[filled].desc,  128, "Unlock the %s.", WpnName(pick));
            filled++;
            continue;
        }
        try_upgrade:
        if (g_weaponCount > 0) {
            int slot = rand() % g_weaponCount;
            if (g_weapons[slot].level >= 8) continue;
            bool dup = false;
            for (int j = 0; j < filled; j++)
                if (g_opts[j].type == 1 && g_opts[j].weaponSlot == slot) { dup = true; break; }
            if (dup) continue;

            g_opts[filled].type       = 1;
            g_opts[filled].weaponSlot = slot;
            snprintf(g_opts[filled].label, 64,  "%s Lv.%d", WpnName(g_weapons[slot].id), g_weapons[slot].level + 1);
            snprintf(g_opts[filled].desc,  128, "Upgrade your %s (damage +25%%, cooldown -8%%).", WpnName(g_weapons[slot].id));
            filled++;
            continue;
        }
        // Fallback: another stat
        for (int s = 0; s < 3; s++) {
            if (!used[s]) {
                g_opts[filled].type    = 0;
                g_opts[filled].statIdx = s;
                strncpy(g_opts[filled].label, statLabels[s], 63);
                strncpy(g_opts[filled].desc,  statDescs[s],  127);
                used[s] = true;
                filled++;
                break;
            }
        }
    }
    // Safety: fill any remaining with stat options
    while (filled < 3) {
        g_opts[filled].type    = 0;
        g_opts[filled].statIdx = 0;
        strncpy(g_opts[filled].label, statLabels[0], 63);
        strncpy(g_opts[filled].desc,  statDescs[0],  127);
        filled++;
    }
}

static void ApplyUpgrade(int choice) {
    UpgradeOption *opt = &g_opts[choice];
    switch (opt->type) {
        case 0: // stat
            switch (opt->statIdx) {
                case 0: g_player.maxHp *= 1.2f; g_player.hp = fminf(g_player.hp + g_player.maxHp * 0.2f, g_player.maxHp); break;
                case 1: g_player.speed *= 1.1f; break;
                case 2: g_player.hp = fminf(g_player.hp + 30.0f, g_player.maxHp); break;
            }
            break;
        case 1: // upgrade weapon
            if (opt->weaponSlot < g_weaponCount) {
                g_weapons[opt->weaponSlot].level++;
                g_weapons[opt->weaponSlot].cooldown = WpnCooldown(
                    g_weapons[opt->weaponSlot].id, g_weapons[opt->weaponSlot].level);
            }
            break;
        case 2: // new weapon
            if (g_weaponCount < MAX_WEAPONS) {
                g_weapons[g_weaponCount].id       = opt->newWpn;
                g_weapons[g_weaponCount].level    = 1;
                g_weapons[g_weaponCount].cooldown = WpnCooldown(opt->newWpn, 1);
                g_weapons[g_weaponCount].timer    = 0;
                g_weapons[g_weaponCount].angle    = 0;
                g_weaponCount++;
            }
            break;
    }
    g_pendingLevels--;
    if (g_pendingLevels > 0)
        BuildUpgradeOptions();
    else
        g_state = STATE_PLAYING;
}

// ============================================================
// Init / Reset
// ============================================================
static void InitGame(void) {
    srand((unsigned)time(NULL));

    memset(g_enemies, 0, sizeof(g_enemies));
    memset(g_bullets, 0, sizeof(g_bullets));
    memset(g_gems,    0, sizeof(g_gems));
    memset(g_weapons, 0, sizeof(g_weapons));

    g_player.pos      = (Vector2){0, 0};
    g_player.hp       = 100.0f;
    g_player.maxHp    = 100.0f;
    g_player.speed    = 150.0f;
    g_player.xp       = 0;
    g_player.xpNext   = 10.0f;
    g_player.level    = 1;
    g_player.iframes  = 0;
    g_player.alive    = 1;

    g_weapons[0].id       = WPN_WAND;
    g_weapons[0].level    = 1;
    g_weapons[0].cooldown = WpnCooldown(WPN_WAND, 1);
    g_weaponCount = 1;

    g_time          = 0;
    g_kills         = 0;
    g_spawnTimer    = 0;
    g_spawnInterval = 1.5f;
    g_pendingLevels = 0;
    g_state         = STATE_PLAYING;

    g_cam.target   = g_player.pos;
    g_cam.offset   = (Vector2){ SCREEN_W * 0.5f, SCREEN_H * 0.5f };
    g_cam.rotation = 0;
    g_cam.zoom     = 1.0f;
}

// ============================================================
// Update
// ============================================================
static void Update(float dt) {
    if (g_state == STATE_LEVELUP || g_state == STATE_DEAD || g_state == STATE_WIN)
        return;

    g_time += dt;
    if (g_time >= WIN_TIME) { g_state = STATE_WIN; return; }

    // Player movement
    Vector2 move = {0, 0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    move.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  move.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  move.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1;
    float mlen = V2Len(move);
    if (mlen > 0.0f) {
        g_player.pos.x += (move.x / mlen) * g_player.speed * dt;
        g_player.pos.y += (move.y / mlen) * g_player.speed * dt;
    }
    if (g_player.iframes > 0) g_player.iframes -= dt;

    // Enemy spawning
    g_spawnInterval = fmaxf(0.12f, 1.5f - g_time * 0.0022f);
    g_spawnTimer   += dt;
    int batch = 1 + (int)(g_time / 60.0f);
    if (batch > 10) batch = 10;
    while (g_spawnTimer >= g_spawnInterval) {
        g_spawnTimer -= g_spawnInterval;
        for (int b = 0; b < batch; b++) SpawnEnemy();
    }

    // Update weapons
    for (int i = 0; i < g_weaponCount; i++) {
        WeaponSlot *ws = &g_weapons[i];
        if (ws->id == WPN_GARLIC) { UpdateGarlic(ws, dt); continue; }
        if (ws->id == WPN_ORB)    { UpdateOrb(ws, dt);    continue; }
        ws->timer -= dt;
        if (ws->timer <= 0.0f) {
            ws->timer = ws->cooldown;
            switch (ws->id) {
                case WPN_WAND:  FireWand(ws);  break;
                case WPN_AXE:   FireAxe(ws);   break;
                case WPN_CROSS: FireCross(ws); break;
                default: break;
            }
        }
    }

    // Update bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *b = &g_bullets[i];
        if (!b->alive) continue;
        b->pos.x   += b->vel.x * dt;
        b->pos.y   += b->vel.y * dt;
        b->lifetime -= dt;
        if (b->lifetime <= 0) { b->alive = 0; continue; }
        // Axe arc: gravity
        if (b->source == WPN_AXE) b->vel.y += 650.0f * dt;
        // Hit enemies
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!g_enemies[j].alive) continue;
            if (V2Dist(b->pos, g_enemies[j].pos) < b->radius + g_enemies[j].radius) {
                g_enemies[j].hp -= b->damage;
                if (b->pierce <= 0) { b->alive = 0; break; }
                b->pierce--;
            }
        }
    }

    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &g_enemies[i];
        if (!e->alive) continue;
        if (e->hp <= 0) {
            e->alive = 0;
            g_kills++;
            int gi = FreeGem();
            if (gi >= 0) {
                g_gems[gi].pos   = e->pos;
                g_gems[gi].value = e->xpDrop;
                g_gems[gi].alive = 1;
            }
            continue;
        }
        // Move toward player
        Vector2 dir = { g_player.pos.x - e->pos.x, g_player.pos.y - e->pos.y };
        float d = V2Len(dir);
        if (d > 0.01f) {
            e->pos.x += (dir.x / d) * e->speed * dt;
            e->pos.y += (dir.y / d) * e->speed * dt;
        }
        // Damage player on contact
        if (V2Dist(e->pos, g_player.pos) < e->radius + 16.0f) {
            e->attackCooldown -= dt;
            if (e->attackCooldown <= 0 && g_player.iframes <= 0) {
                g_player.hp -= e->damage;
                g_player.iframes = 0.5f;
                e->attackCooldown = 1.0f;
                if (g_player.hp <= 0) {
                    g_player.hp    = 0;
                    g_player.alive = 0;
                    g_state        = STATE_DEAD;
                    return;
                }
            }
        }
    }

    // Collect XP gems
    for (int i = 0; i < MAX_GEMS; i++) {
        if (!g_gems[i].alive) continue;
        float d = V2Dist(g_player.pos, g_gems[i].pos);
        // Magnetic pull
        if (d < 130.0f && d > 35.0f) {
            Vector2 dir2 = { g_player.pos.x - g_gems[i].pos.x, g_player.pos.y - g_gems[i].pos.y };
            float spd = 220.0f;
            g_gems[i].pos.x += (dir2.x / d) * spd * dt;
            g_gems[i].pos.y += (dir2.y / d) * spd * dt;
        }
        if (d < 35.0f) {
            g_player.xp += g_gems[i].value;
            g_gems[i].alive = 0;
            // Level up?
            while (g_player.xp >= g_player.xpNext) {
                g_player.xp    -= g_player.xpNext;
                g_player.xpNext *= 1.4f;
                g_player.level++;
                g_pendingLevels++;
                g_state = STATE_LEVELUP;
                BuildUpgradeOptions();
                return; // handle one level-up at a time
            }
        }
    }

    // Camera follows player
    g_cam.target = g_player.pos;
}

// ============================================================
// Draw
// ============================================================
static void DrawCheckerboard(void) {
    int gs = 64;
    int tx = (int)(g_cam.target.x / gs) - SCREEN_W / (gs * 2) - 2;
    int ty = (int)(g_cam.target.y / gs) - SCREEN_H / (gs * 2) - 2;
    int cntX = SCREEN_W / gs + 5;
    int cntY = SCREEN_H / gs + 5;
    for (int y = ty; y < ty + cntY; y++) {
        for (int x = tx; x < tx + cntX; x++) {
            Color c = ((x + y) & 1) ? (Color){28,28,28,255} : (Color){22,22,22,255};
            DrawRectangle(x * gs, y * gs, gs, gs, c);
        }
    }
}

static void DrawHUD(void) {
    // HP bar
    int bw = 220, bh = 22;
    DrawRectangle(18, 18, bw, bh, DARKGRAY);
    DrawRectangle(18, 18, (int)(bw * (g_player.hp / g_player.maxHp)), bh, RED);
    DrawRectangleLines(18, 18, bw, bh, WHITE);
    DrawText(TextFormat("HP %d / %d", (int)g_player.hp, (int)g_player.maxHp), 24, 21, 14, WHITE);

    // XP bar
    int xw = 220, xh = 12;
    DrawRectangle(18, 44, xw, xh, DARKGRAY);
    DrawRectangle(18, 44, (int)(xw * (g_player.xp / g_player.xpNext)), xh, BLUE);
    DrawRectangleLines(18, 44, xw, xh, WHITE);
    DrawText(TextFormat("Lv.%d", g_player.level), 244, 42, 16, GOLD);

    // Timer
    int mins = (int)(g_time / 60);
    int secs = (int)(g_time) % 60;
    const char *ts = TextFormat("%02d:%02d", mins, secs);
    DrawText(ts, SCREEN_W / 2 - MeasureText(ts, 24) / 2, 14, 24, WHITE);

    // Kills
    DrawText(TextFormat("Kills: %d", g_kills), SCREEN_W - 130, 14, 18, WHITE);

    // Weapon list
    for (int i = 0; i < g_weaponCount; i++) {
        DrawText(TextFormat("%s Lv%d", WpnName(g_weapons[i].id), g_weapons[i].level),
            18, 72 + i * 20, 14, YELLOW);
    }

    // Controls hint (bottom-left)
    DrawText("WASD to move  |  survive 15 min", 18, SCREEN_H - 24, 14, DARKGRAY);
}

static void DrawLevelUpMenu(void) {
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){0, 0, 0, 170});

    const char *title = TextFormat("LEVEL UP!  Level %d", g_player.level);
    DrawText(title, SCREEN_W / 2 - MeasureText(title, 38) / 2, 110, 38, GOLD);

    int cw = 270, ch = 190, gap = 28;
    int total = 3 * cw + 2 * gap;
    int sx = SCREEN_W / 2 - total / 2;
    int cy = 200;

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; i++) {
        int cx = sx + i * (cw + gap);
        Rectangle r = { (float)cx, (float)cy, (float)cw, (float)ch };
        bool hov = CheckCollisionPointRec(mouse, r);

        DrawRectangleRec(r, hov ? (Color){55,55,80,255} : (Color){28,28,50,255});
        DrawRectangleLinesEx(r, 2, hov ? GOLD : GRAY);

        DrawText(TextFormat("[%d]", i + 1), cx + 10, cy + 10, 16, GRAY);

        int lw = MeasureText(g_opts[i].label, 20);
        DrawText(g_opts[i].label, cx + cw / 2 - lw / 2, cy + 38, 20, WHITE);

        // Simple line-wrapped description
        DrawText(g_opts[i].desc, cx + 12, cy + 76, 14, LIGHTGRAY);

        if ((hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
            (i == 0 && IsKeyPressed(KEY_ONE))   ||
            (i == 1 && IsKeyPressed(KEY_TWO))   ||
            (i == 2 && IsKeyPressed(KEY_THREE)))
            ApplyUpgrade(i);
    }

    DrawText("Click a card or press 1 / 2 / 3",
        SCREEN_W / 2 - MeasureText("Click a card or press 1 / 2 / 3", 18) / 2,
        cy + ch + 22, 18, GRAY);
}

static void DrawOrbWeapons(void) {
    for (int i = 0; i < g_weaponCount; i++) {
        if (g_weapons[i].id != WPN_ORB) continue;
        WeaponSlot *ws = &g_weapons[i];
        int   count = 1 + ws->level / 2;
        float range = 80.0f + ws->level * 12.0f;
        for (int c = 0; c < count; c++) {
            float a = ws->angle + (float)c * (2.0f * PI / count);
            Vector2 op = {
                g_player.pos.x + cosf(a) * range,
                g_player.pos.y + sinf(a) * range,
            };
            DrawCircleV(op, 12, SKYBLUE);
            DrawCircleLines((int)op.x, (int)op.y, 12, WHITE);
        }
    }
}

static void DrawGarlicAura(void) {
    for (int i = 0; i < g_weaponCount; i++) {
        if (g_weapons[i].id != WPN_GARLIC) continue;
        float r = 85.0f + g_weapons[i].level * 15.0f;
        DrawCircle((int)g_player.pos.x, (int)g_player.pos.y, r, (Color){200,200,60,20});
        DrawCircleLines((int)g_player.pos.x, (int)g_player.pos.y, (int)r, (Color){200,200,60,100});
    }
}

static void DrawEndScreen(void) {
    bool dead = (g_state == STATE_DEAD);
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){0, 0, 0, 175});

    const char *headline = dead ? "YOU DIED" : "YOU SURVIVED!";
    Color hcol = dead ? RED : GOLD;
    DrawText(headline, SCREEN_W / 2 - MeasureText(headline, 64) / 2, SCREEN_H / 2 - 80, 64, hcol);

    int mins = (int)(g_time / 60);
    int secs = (int)(g_time) % 60;
    const char *sub = TextFormat("Time: %02d:%02d  |  Kills: %d  |  Level: %d",
        mins, secs, g_kills, g_player.level);
    DrawText(sub, SCREEN_W / 2 - MeasureText(sub, 22) / 2, SCREEN_H / 2 + 10, 22, WHITE);

    const char *hint = "Press R to restart";
    DrawText(hint, SCREEN_W / 2 - MeasureText(hint, 20) / 2, SCREEN_H / 2 + 60, 20, GRAY);
}

static void Draw(void) {
    BeginDrawing();
    ClearBackground((Color){10, 10, 10, 255});

    BeginMode2D(g_cam);
        DrawCheckerboard();

        // XP gems
        for (int i = 0; i < MAX_GEMS; i++) {
            if (!g_gems[i].alive) continue;
            DrawCircleV(g_gems[i].pos, 6, BLUE);
            DrawCircleLines((int)g_gems[i].pos.x, (int)g_gems[i].pos.y, 6, SKYBLUE);
        }

        DrawGarlicAura();

        // Enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy *e = &g_enemies[i];
            if (!e->alive) continue;
            DrawCircleV(e->pos, e->radius, e->color);
            // HP bar
            float bw = e->radius * 2.0f;
            DrawRectangle((int)(e->pos.x - e->radius), (int)(e->pos.y - e->radius - 9), (int)bw, 4, DARKGRAY);
            DrawRectangle((int)(e->pos.x - e->radius), (int)(e->pos.y - e->radius - 9),
                (int)(bw * (e->hp / e->maxHp)), 4, RED);
        }

        // Bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            Bullet *b = &g_bullets[i];
            if (!b->alive) continue;
            DrawCircleV(b->pos, b->radius, b->color);
        }

        DrawOrbWeapons();

        // Player (flicker when invincible)
        bool showPlayer = (g_player.iframes <= 0) || ((int)(g_player.iframes * 12) & 1);
        if (showPlayer) {
            DrawCircleV(g_player.pos, 16, LIME);
            DrawCircleLines((int)g_player.pos.x, (int)g_player.pos.y, 16, WHITE);
        }
    EndMode2D();

    DrawHUD();

    if (g_state == STATE_LEVELUP) DrawLevelUpMenu();
    if (g_state == STATE_DEAD || g_state == STATE_WIN) DrawEndScreen();

    EndDrawing();
}

// ============================================================
// Main
// ============================================================
int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Vampire Survivors");
    SetTargetFPS(TARGET_FPS);

    InitGame();

    while (!WindowShouldClose()) {
        if ((g_state == STATE_DEAD || g_state == STATE_WIN) && IsKeyPressed(KEY_R))
            InitGame();
        Update(GetFrameTime());
        Draw();
    }

    CloseWindow();
    return 0;
}
