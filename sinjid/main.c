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

// ── Demo / screenshot mode ────────────────────────────────────────────────
static void demo_setup_combat(void)
{
    player_init(&g_player, "Hero", CLASS_WARRIOR);
    g_player.level        = 5;
    g_player.gold         = 340;
    g_player.current_life = 95;
    g_player.base.max_life = 120;
    g_player.current_mana = 30;
    g_player.base.max_mana = 40;
    g_player.equip[EQUIP_WEAPON]    = 3;  // Silver Knife
    g_player.equip[EQUIP_ARMOR]     = 11; // Leather Armour
    g_player.equip[EQUIP_ACCESSORY] = 19; // Bandana
    player_add_item(&g_player, 23); // Small Potion
    player_add_item(&g_player, 24); // Large Potion
    combat_start(GW_HUMAN, 2);     // vs Raider
    g_combat.player_turn  = true;
    g_combat.log_count    = 0;
    combat_log("Battle starts: Raider");
    combat_log("Hero attacks for 18 damage.");
    combat_log("Raider attacks for 12 damage.");
    combat_log("Hero uses Power Strike for 29!");
}

static void demo_setup_hub(void)
{
    player_init(&g_player, "Hero", CLASS_WARRIOR);
    g_player.level        = 5;
    g_player.gold         = 340;
    g_player.xp           = 80;
    g_player.xp_to_next   = 130;
    g_player.current_life = 95;
    g_player.base.max_life = 120;
    g_player.current_mana = 30;
    g_player.base.max_mana = 40;
    g_player.equip[EQUIP_WEAPON]    = 3;
    g_player.equip[EQUIP_ARMOR]     = 11;
    g_player.gw_progress[GW_HUMAN]  = 2;
}

static void run_demo(void)
{
    // 1 – Title
    g_scene = SCENE_TITLE;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_title(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_01_title.png");
    WaitTime(0.05);

    // 2 – Class select
    g_scene = SCENE_CLASS_SELECT;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_class_select(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_02_class.png");
    WaitTime(0.05);

    // 3 – Hub
    demo_setup_hub();
    g_scene = SCENE_HUB;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_hub(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_03_hub.png");
    WaitTime(0.05);

    // 4 – Gateway select
    g_combat.gateway = GW_HUMAN;
    g_scene = SCENE_GATEWAY_SELECT;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_gateway_select(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_04_gateway.png");
    WaitTime(0.05);

    // 5 – Combat
    demo_setup_combat();
    g_scene = SCENE_COMBAT;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_combat(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_05_combat.png");
    WaitTime(0.05);

    // 6 – Shop
    demo_setup_hub();
    g_shop.item_ids = g_shop_basic;
    g_shop.count    = g_shop_basic_count;
    g_shop.selected = 0;
    g_scene = SCENE_SHOP;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_shop(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_06_shop.png");
    WaitTime(0.05);

    // 7 – Inventory
    demo_setup_hub();
    player_add_item(&g_player, 4);  // Spiked Axe
    player_add_item(&g_player, 23); // Small Potion
    g_scene = SCENE_INVENTORY;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_inventory(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_07_inventory.png");
    WaitTime(0.05);

    // 8 – Skills
    demo_setup_hub();
    g_player.skill_points = 2;
    g_player.skill_level[0] = 2;
    g_player.skill_level[1] = 1;
    g_scene = SCENE_SKILLS;
    BeginDrawing(); ClearBackground((Color){15,15,25,255});
    ui_draw_skills(); EndDrawing();
    TakeScreenshot("/tmp/sinjid_08_skills.png");
    WaitTime(0.05);
}

// ── Main ──────────────────────────────────────────────────────────────────
int main(int argc, char **argv)
{
    bool demo_mode = (argc > 1 && strcmp(argv[1], "--demo") == 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // disable ESC quit so we can use it in menus

    if (demo_mode) {
        run_demo();
        CloseWindow();
        return 0;
    }

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
