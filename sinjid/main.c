#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "ui.h"
#include "player.h"
#include "combat.h"
#include "hub.h"
#include "data.h"
#include "anim.h"
#include "audio.h"

// ── Globals ───────────────────────────────────────────────────────────────
Scene        g_scene   = SCENE_TITLE;
Player       g_player;
CombatState  g_combat;
ShopState    g_shop;
LevelUpState g_lvlup;
int          g_hover_btn = -1;

// ── Layered rendering ─────────────────────────────────────────────────────
// rt_static: cached scene pixels (only redrawn when g_scene_dirty).
//   The combat scene is excluded from static caching because of continuous
//   animation; all other scenes cache their heavy text/panel draw calls here.
// rt_fade: full-screen overlay used for scene transition fades.

static RenderTexture2D rt_static;
static RenderTexture2D rt_fade;

// Which scenes are purely static (no live elements other than the fade
// overlay and button hovers that we composite separately)
static bool is_static_scene(Scene s)
{
    // Combat always redraws (idle bob, bar lerp, hit flash, enemy AI tick)
    return s != SCENE_COMBAT;
}

static void draw_scene(void)
{
    ClearBackground((Color){ 15, 15, 25, 255 });
    switch (g_scene) {
        case SCENE_TITLE:          ui_draw_title();          break;
        case SCENE_CLASS_SELECT:   ui_draw_class_select();   break;
        case SCENE_HUB:            ui_draw_hub();            break;
        case SCENE_COMBAT:         ui_draw_combat();         break;
        case SCENE_INVENTORY:      ui_draw_inventory();      break;
        case SCENE_SHOP:           ui_draw_shop();           break;
        case SCENE_SKILLS:         ui_draw_skills();         break;
        case SCENE_LEVEL_UP:       ui_draw_level_up();       break;
        case SCENE_GATEWAY_SELECT: ui_draw_gateway_select(); break;
        case SCENE_GAME_OVER:      ui_draw_game_over();      break;
        case SCENE_VICTORY:        ui_draw_victory();        break;
    }
}

// ── Demo screenshot mode ──────────────────────────────────────────────────
static void demo_setup_combat(void)
{
    player_init(&g_player, "Hero", CLASS_WARRIOR);
    g_player.level = 5; g_player.gold = 340;
    g_player.current_life = 95; g_player.base.max_life = 120;
    g_player.current_mana = 30; g_player.base.max_mana = 40;
    g_player.equip[EQUIP_WEAPON] = 3; g_player.equip[EQUIP_ARMOR] = 11;
    player_add_item(&g_player, 23); player_add_item(&g_player, 24);
    combat_start(GW_HUMAN, 2);
    g_combat.player_turn = true; g_combat.log_count = 0;
    combat_log("Battle starts: Seer");
    combat_log("Hero attacks for 18 damage.");
    combat_log("Seer attacks for 12 damage.");
    combat_log("Hero uses Power Strike for 29!");
    anim_combat_reset();
}

static void demo_setup_hub(void)
{
    player_init(&g_player, "Hero", CLASS_WARRIOR);
    g_player.level = 5; g_player.gold = 340;
    g_player.xp = 80; g_player.xp_to_next = 130;
    g_player.current_life = 95; g_player.base.max_life = 120;
    g_player.current_mana = 30; g_player.base.max_mana = 40;
    g_player.equip[EQUIP_WEAPON] = 3; g_player.equip[EQUIP_ARMOR] = 11;
    g_player.gw_progress[GW_HUMAN] = 2;
}

static void run_demo(void)
{
    struct { Scene sc; const char *file; void (*setup)(void); } shots[] = {
        { SCENE_TITLE,          "/tmp/sinjid_01_title.png",   NULL             },
        { SCENE_CLASS_SELECT,   "/tmp/sinjid_02_class.png",   NULL             },
        { SCENE_HUB,            "/tmp/sinjid_03_hub.png",     demo_setup_hub   },
        { SCENE_GATEWAY_SELECT, "/tmp/sinjid_04_gateway.png", demo_setup_hub   },
        { SCENE_COMBAT,         "/tmp/sinjid_05_combat.png",  demo_setup_combat},
        { SCENE_SHOP,           "/tmp/sinjid_06_shop.png",    demo_setup_hub   },
        { SCENE_INVENTORY,      "/tmp/sinjid_07_inventory.png",demo_setup_hub  },
        { SCENE_SKILLS,         "/tmp/sinjid_08_skills.png",  demo_setup_hub   },
    };

    for (int i = 0; i < (int)(sizeof(shots)/sizeof(shots[0])); i++) {
        if (shots[i].setup) shots[i].setup();
        if (shots[i].sc == SCENE_GATEWAY_SELECT) g_combat.gateway = GW_HUMAN;
        if (shots[i].sc == SCENE_SHOP) {
            g_shop.item_ids = g_shop_basic; g_shop.count = g_shop_basic_count;
        }
        if (shots[i].sc == SCENE_INVENTORY) {
            player_add_item(&g_player, 4); player_add_item(&g_player, 23);
        }
        if (shots[i].sc == SCENE_SKILLS) {
            g_player.skill_points = 2;
            g_player.skill_level[0] = 2;
        }
        g_scene = shots[i].sc;
        BeginDrawing(); draw_scene(); EndDrawing();
        TakeScreenshot(shots[i].file);
        WaitTime(0.05);
    }
}

// ── Main ──────────────────────────────────────────────────────────────────
int main(int argc, char **argv)
{
    bool demo_mode = (argc > 1 && strcmp(argv[1], "--demo") == 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    audio_init();
    ui_init();

    rt_static = LoadRenderTexture(SCREEN_W, SCREEN_H);
    rt_fade   = LoadRenderTexture(SCREEN_W, SCREEN_H);

    // Pre-fill fade texture with solid black
    BeginTextureMode(rt_fade);
        ClearBackground(BLACK);
    EndTextureMode();

    if (demo_mode) {
        run_demo();
        UnloadRenderTexture(rt_static);
        UnloadRenderTexture(rt_fade);
        ui_close();
        audio_close();
        CloseWindow();
        return 0;
    }

    // Start with a fade-in from black
    g_fade.fade_alpha = 1.0f;
    g_fade.fading_out = false;
    g_fade.active     = true;
    g_scene_dirty     = true;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // ESC to go back
        if (IsKeyPressed(KEY_ESCAPE)) {
            switch (g_scene) {
                case SCENE_INVENTORY:
                case SCENE_SHOP:
                case SCENE_SKILLS:
                case SCENE_GATEWAY_SELECT:
                    anim_fade_to(SCENE_HUB);
                    break;
                default: break;
            }
        }

        // Update animations
        anim_update(dt);

        // Detect scene change → mark dirty
        static Scene last_scene = -1;
        if (g_scene != last_scene) {
            g_scene_dirty = true;
            last_scene    = g_scene;
        }

        // ── Static layer: redraw only when dirty ──────────────────────────
        if (is_static_scene(g_scene) && g_scene_dirty) {
            BeginTextureMode(rt_static);
                draw_scene();
            EndTextureMode();
            g_scene_dirty = false;
        }

        // ── Composite to screen ───────────────────────────────────────────
        BeginDrawing();
            ClearBackground((Color){15,15,25,255});

            if (is_static_scene(g_scene)) {
                // Blit cached static frame
                DrawTextureRec(rt_static.texture,
                    (Rectangle){0, 0, SCREEN_W, -SCREEN_H},
                    (Vector2){0, 0}, WHITE);
            } else {
                // Combat: always redraw live
                draw_scene();
            }

            // Scene-fade overlay
            if (g_fade.active || g_fade.fade_alpha > 0.0f) {
                unsigned char a = (unsigned char)(g_fade.fade_alpha * 255.0f);
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){0,0,0,a});
            }

            // Debug overlay (F1)
            static bool debug = false;
            if (IsKeyPressed(KEY_F1)) debug = !debug;
            if (debug) {
                DrawText(TextFormat("Scene:%d Dirty:%d Fade:%.2f Level:%d XP:%d/%d",
                    g_scene, g_scene_dirty, g_fade.fade_alpha,
                    g_player.level, g_player.xp, g_player.xp_to_next),
                    4, 4, 15, LIME);
                DrawText(TextFormat("Gold:%d HP:%d/%d MP:%d/%d EFlash:%.2f PFlash:%.2f",
                    g_player.gold,
                    g_player.current_life, player_effective_life(),
                    g_player.current_mana, player_effective_mana(),
                    g_canim.enemy_flash, g_canim.player_flash),
                    4, 22, 15, LIME);
            }

        EndDrawing();
    }

    UnloadRenderTexture(rt_static);
    UnloadRenderTexture(rt_fade);
    ui_close();
    audio_close();
    CloseWindow();
    return 0;
}
