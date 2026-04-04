#include "ui_internal.h"

static int s_shop_sel = -1;

void ui_draw_shop(void)
{
    txt(20, 16, 26, C_GOLD, "Shop");
    txt(20, 50, 19, C_GOLD, TextFormat("Your Gold: %d", g_ctx.player.gold));

    Rectangle list_panel = { 20, 80, 380, SCREEN_H - 140 };
    ui_panel(list_panel, C_PANEL, C_BORDER);
    for (int i = 0; i < g_ctx.shop.count; i++) {
        int id = g_ctx.shop.item_ids[i];
        const ItemDef *it = &g_items[id];
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s  [%dg]", it->name, it->price);
        Rectangle ir = { 34, (float)(94 + i * 48), 352, 40 };
        bool sel = (s_shop_sel == i);
        DrawRectangleRec(ir, sel ? (Color){60,60,110,255} : C_BTN);
        DrawRectangleLinesEx(ir, 1, C_BORDER);
        Color tc = (g_ctx.player.gold >= it->price) ? C_WHITE : C_DIM;
        txt((int)ir.x + 8, (int)ir.y + 12, 15, tc, lbl);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, ir) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            s_shop_sel = (sel ? -1 : i);
            snd_click();
        }
    }

    Rectangle det = { 420, 80, SCREEN_W - 440, SCREEN_H - 140 };
    ui_panel(det, C_PANEL, C_BORDER);

    if (s_shop_sel >= 0 && s_shop_sel < g_ctx.shop.count) {
        int id = g_ctx.shop.item_ids[s_shop_sel];
        const ItemDef *it = &g_items[id];
        txt(436, 96, 22, C_GOLD, it->name);
        txt(436, 124, 17, C_GOLD, TextFormat("Price: %d gold", it->price));

        int dy = 158;
        static const char *type_names[] = {"None","Weapon","Armor","Accessory","Consumable"};
        txt(436, dy, 15, C_DIM, type_names[it->type]); dy += 28;
        if (it->bonus_life) { txt(436, dy, 15, C_HP,    TextFormat("+%d Life",    it->bonus_life)); dy+=22; }
        if (it->bonus_mana) { txt(436, dy, 15, C_MP,    TextFormat("+%d Mana",    it->bonus_mana)); dy+=22; }
        if (it->bonus_str)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Strength",it->bonus_str));  dy+=22; }
        if (it->bonus_spd)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Speed",   it->bonus_spd));  dy+=22; }
        if (it->bonus_def)  { txt(436, dy, 15, C_WHITE, TextFormat("+%d Defense", it->bonus_def));  dy+=22; }

        bool can = (g_ctx.player.gold >= it->price);
        Rectangle buy_btn = { 436, (float)(dy + 30), 140, 48 };
        if (ui_button(buy_btn, "Buy", can)) {
            if (player_add_item(&g_ctx.player, id)) {
                g_ctx.player.gold -= it->price;
                g_ctx.scene_dirty = true;
            }
        }
        if (!can) txt(436, (float)(dy + 84), 15, C_RED, "Not enough gold!");
    } else {
        txt(436, 160, 17, C_DIM, "Select an item.");
    }

    Rectangle back = { 20, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_shop_sel = -1;
        anim_fade_to(SCENE_HUB);
    }
}
