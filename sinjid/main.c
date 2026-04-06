#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "config.h"
#include "scene.h"
#include "ui.h"
#include "player.h"
#include "combat.h"
#include "hub.h"
#include "data.h"
#include "anim.h"
#include "audio.h"
#include "save.h"
#include "events.h"
#include "update.h"

// ── Single global game context ────────────────────────────────────────────
GameCtx g_ctx;

// ── Layered rendering ─────────────────────────────────────────────────────
// rt_static: cached scene pixels (only redrawn when g_ctx.scene_dirty).
//   The combat scene is excluded from static caching because of continuous
//   animation; all other scenes cache their heavy text/panel draw calls here.

/* Single render texture: all scenes draw here at SCREEN_W×SCREEN_H,
   then it is blitted letterboxed to the actual window. */
static RenderTexture2D rt_game;

static void draw_scene(void)
{
    ClearBackground((Color){ 15, 15, 25, 255 });
    switch (G_SCENE) {
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
        default:                   break;
    }
}

// ── Demo screenshot mode ──────────────────────────────────────────────────
static void demo_setup_combat(void)
{
    player_init(&g_ctx.player, "Hero", CLASS_WARRIOR);
    g_ctx.player.level = 5; g_ctx.player.gold = 340;
    g_ctx.player.current_life = 95; g_ctx.player.base.max_life = 120;
    g_ctx.player.current_mana = 30; g_ctx.player.base.max_mana = 40;
    g_ctx.player.equip[EQUIP_WEAPON] = 3; g_ctx.player.equip[EQUIP_ARMOR] = 11;
    player_add_item(&g_ctx.player, 23); player_add_item(&g_ctx.player, 24);
    combat_start(GW_HUMAN, 2);
    g_ctx.combat.player_turn = true; g_ctx.combat.log_count = 0;
    combat_log("Battle starts: Seer");
    combat_log("Hero attacks for 18 damage.");
    combat_log("Seer attacks for 12 damage.");
    combat_log("Hero uses Power Strike for 29!");
    anim_combat_reset();
}

static void demo_setup_hub(void)
{
    player_init(&g_ctx.player, "Hero", CLASS_WARRIOR);
    g_ctx.player.level = 5; g_ctx.player.gold = 340;
    g_ctx.player.xp = 80; g_ctx.player.xp_to_next = 130;
    g_ctx.player.current_life = 95; g_ctx.player.base.max_life = 120;
    g_ctx.player.current_mana = 30; g_ctx.player.base.max_mana = 40;
    g_ctx.player.equip[EQUIP_WEAPON] = 3; g_ctx.player.equip[EQUIP_ARMOR] = 11;
    g_ctx.player.gw_progress[GW_HUMAN] = 2;
}

static void run_demo(void)
{
    struct { Scene sc; const char *file; void (*setup)(void); } shots[] = {
        { SCENE_TITLE,          "/tmp/sinjid_01_title.png",    NULL             },
        { SCENE_CLASS_SELECT,   "/tmp/sinjid_02_class.png",    NULL             },
        { SCENE_HUB,            "/tmp/sinjid_03_hub.png",      demo_setup_hub   },
        { SCENE_GATEWAY_SELECT, "/tmp/sinjid_04_gateway.png",  demo_setup_hub   },
        { SCENE_COMBAT,         "/tmp/sinjid_05_combat.png",   demo_setup_combat},
        { SCENE_SHOP,           "/tmp/sinjid_06_shop.png",     demo_setup_hub   },
        { SCENE_INVENTORY,      "/tmp/sinjid_07_inventory.png",demo_setup_hub   },
        { SCENE_SKILLS,         "/tmp/sinjid_08_skills.png",   demo_setup_hub   },
    };

    for (int i = 0; i < (int)(sizeof(shots)/sizeof(shots[0])); i++) {
        if (shots[i].setup) shots[i].setup();
        if (shots[i].sc == SCENE_GATEWAY_SELECT) g_ctx.combat.gateway = GW_HUMAN;
        if (shots[i].sc == SCENE_SHOP) {
            g_ctx.shop.item_ids = g_shop_basic;
            g_ctx.shop.count    = g_shop_basic_count;
        }
        if (shots[i].sc == SCENE_INVENTORY) {
            player_add_item(&g_ctx.player, 4);
            player_add_item(&g_ctx.player, 23);
        }
        if (shots[i].sc == SCENE_SKILLS) {
            g_ctx.player.skill_points = 2;
            g_ctx.player.skill_level[0] = 2;
        }
        scene_replace(&g_ctx.scenes, shots[i].sc);
        BeginDrawing(); draw_scene(); EndDrawing();
        TakeScreenshot(shots[i].file);
        WaitTime(0.05);
    }
}

// ── Main ──────────────────────────────────────────────────────────────────
int main(int argc, char **argv)
{
    bool demo_mode = (argc > 1 && strcmp(argv[1], "--demo") == 0);

    // Config must be loaded before InitWindow so FPS and paths are known.
    // window_w/h == 0 means "auto-size from monitor" (the default).
    config_init_defaults();
    config_load(CONFIG_PATH);

    // Open with a safe placeholder size; resize below once we can query the monitor
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, GAME_TITLE);
    SetTargetFPS(g_config.fps);
    SetExitKey(KEY_NULL);

    // ── Auto-size window to 80% of the current monitor ───────────────────
    if (g_config.window_w == 0 || g_config.window_h == 0) {
        int mon   = GetCurrentMonitor();
        int mon_w = GetMonitorWidth(mon);
        int mon_h = GetMonitorHeight(mon);
        // Maintain SCREEN_W:SCREEN_H aspect ratio at ~80% of monitor height
        float scale = 0.80f;
        int w = (int)(mon_h * scale) * SCREEN_W / SCREEN_H;
        int h = (int)(mon_h * scale);
        if (w > (int)(mon_w * scale)) {   // clamp to monitor width
            w = (int)(mon_w * scale);
            h = w * SCREEN_H / SCREEN_W;
        }
        if (w < 1024) { w = 1024; h = 768; }   // minimum
        SetWindowSize(w, h);
        SetWindowPosition((mon_w - w) / 2, (mon_h - h) / 2);
    } else {
        SetWindowSize(g_config.window_w, g_config.window_h);
    }

    // Load game data (defaults first, then override from data/ files)
    data_init_defaults();
    data_load_files(g_config.data_dir);

    // Seed RNG once at startup (combat_start must NOT call srand)
    srand((unsigned)time(NULL));

    audio_init();
    ui_init();

    // Initialize game context
    scene_init(&g_ctx.scenes, SCENE_TITLE);
    evt_init(&g_ctx.events);
    g_ctx.scene_dirty = true;

    rt_game = LoadRenderTexture(SCREEN_W, SCREEN_H);

    if (demo_mode) {
        run_demo();
        UnloadRenderTexture(rt_game);
        ui_close();
        audio_close();
        CloseWindow();
        return 0;
    }

    // Start with a fade-in from black
    g_ctx.fade.fade_alpha = 1.0f;
    g_ctx.fade.fading_out = false;
    g_ctx.fade.active     = true;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // ESC to go back
        if (IsKeyPressed(KEY_ESCAPE)) {
            switch (G_SCENE) {
                case SCENE_INVENTORY:
                case SCENE_SHOP:
                case SCENE_SKILLS:
                case SCENE_GATEWAY_SELECT:
                    anim_fade_to(SCENE_HUB);
                    break;
                default: break;
            }
        }

        // Update game logic (AI timers, etc.) before rendering
        scene_update(dt);

        // Update animations
        anim_update(dt);

        // ── Render scene into fixed-res texture (every frame) ────────────
        // ui_button() both draws and polls for clicks — skipping render for
        // "static" scenes would prevent input from being detected entirely.
        BeginTextureMode(rt_game);
            draw_scene();
        EndTextureMode();
        g_ctx.scene_dirty = false;

        // ── Composite to screen with letterboxing ─────────────────────────
        BeginDrawing();
            ClearBackground(BLACK);   // black bars outside the viewport

            Rectangle vp = game_viewport();
            /* Negative source height flips the texture vertically:
               RenderTextures are stored bottom-up (OpenGL convention). */
            DrawTexturePro(rt_game.texture,
                (Rectangle){0, 0, (float)SCREEN_W, -(float)SCREEN_H},
                vp, (Vector2){0, 0}, 0.0f, WHITE);

            // Scene-fade overlay (covers only the game viewport)
            if (g_ctx.fade.active || g_ctx.fade.fade_alpha > 0.0f) {
                unsigned char a = (unsigned char)(g_ctx.fade.fade_alpha * 255.0f);
                DrawRectangleRec(vp, (Color){0,0,0,a});
            }

            // Debug overlay (F1)
            static bool debug = false;
            if (IsKeyPressed(KEY_F1)) debug = !debug;
            if (debug) {
                DrawText(TextFormat("Scene:%d Dirty:%d Fade:%.2f Level:%d XP:%d/%d",
                    G_SCENE, g_ctx.scene_dirty, g_ctx.fade.fade_alpha,
                    g_ctx.player.level, g_ctx.player.xp, g_ctx.player.xp_to_next),
                    4, 4, 15, LIME);
                DrawText(TextFormat("Gold:%d HP:%d/%d MP:%d/%d EFlash:%.2f PFlash:%.2f",
                    g_ctx.player.gold,
                    g_ctx.player.current_life, player_effective_life(&g_ctx.player),
                    g_ctx.player.current_mana, player_effective_mana(&g_ctx.player),
                    g_ctx.canim.enemy_flash, g_ctx.canim.player_flash),
                    4, 22, 15, LIME);
            }

        EndDrawing();
    }

    // Autosave on clean exit (only if player has actually started a game)
    if (G_SCENE != SCENE_TITLE && G_SCENE != SCENE_CLASS_SELECT &&
        G_SCENE != SCENE_GAME_OVER)
        save_game(&g_ctx.player);

    UnloadRenderTexture(rt_game);
    ui_close();
    audio_close();
    CloseWindow();
    return 0;
}
