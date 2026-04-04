#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "ui.h"
#include "player.h"
#include "combat.h"
#include "hub.h"
#include "data.h"

// ── Global state ──────────────────────────────────────────────────────────
Scene        g_scene   = SCENE_TITLE;
Player       g_player;
CombatState  g_combat;
ShopState    g_shop;
LevelUpState g_lvlup;
int          g_hover_btn = -1;

// ── Main ──────────────────────────────────────────────────────────────────
int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // disable ESC quit so we can use it in menus

    while (!WindowShouldClose()) {
        // ── Handle ESC to go back ─────────────────────────────────────
        if (IsKeyPressed(KEY_ESCAPE)) {
            switch (g_scene) {
                case SCENE_INVENTORY:
                case SCENE_SHOP:
                case SCENE_SKILLS:
                case SCENE_GATEWAY_SELECT:
                    g_scene = SCENE_HUB;
                    break;
                case SCENE_LEVEL_UP:
                    // Don't allow skipping level up
                    break;
                default:
                    break;
            }
        }

        BeginDrawing();
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

        // Debug overlay (toggle with F1)
        static bool debug = false;
        if (IsKeyPressed(KEY_F1)) debug = !debug;
        if (debug) {
            DrawText(TextFormat("Scene: %d  Level: %d  XP: %d/%d",
                g_scene, g_player.level, g_player.xp, g_player.xp_to_next),
                4, 4, 16, LIME);
            DrawText(TextFormat("Gold: %d  HP: %d/%d  MP: %d/%d",
                g_player.gold,
                g_player.current_life, player_effective_life(),
                g_player.current_mana, player_effective_mana()),
                4, 22, 16, LIME);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
