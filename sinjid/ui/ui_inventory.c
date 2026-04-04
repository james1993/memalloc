#include "ui_internal.h"

static int s_inv_selected = -1;

void ui_draw_inventory(void)
{
    txt(20, 16, 26, C_GOLD, "Equipment & Inventory");

    Rectangle eq_panel = { 20, 60, 300, 180 };
    ui_panel(eq_panel, C_PANEL, C_BORDER);
    txt(34, 70, 17, C_WHITE, "Equipped");
    static const char *slot_labels[] = { "Weapon", "Armor ", "Access" };
    for (int s = 0; s < EQUIP_SLOT_COUNT; s++) {
        int eid = g_ctx.player.equip[s];
        const char *nm = (eid >= 0) ? g_items[eid].name : "(empty)";
        Rectangle sr = { 34, (float)(102 + s * 44), 272, 36 };
        if (ui_button(sr, TextFormat("%s: %s", slot_labels[s], nm), true)
            && eid >= 0)
        {
            player_unequip(&g_ctx.player, (EquipSlot)s);
            s_inv_selected = -1;
            g_ctx.scene_dirty = true;
        }
    }

    Rectangle bag_panel = { 20, 260, 300, SCREEN_H - 320 };
    ui_panel(bag_panel, C_PANEL, C_BORDER);
    txt(34, 270, 17, C_WHITE, "Bag");
    int row = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        int id = g_ctx.player.bag_ids[i];
        if (id < 0) continue;
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "%s%s", g_items[id].name,
            (g_items[id].type == ITEM_CONSUMABLE) ?
                TextFormat(" x%d", g_ctx.player.bag_qty[i]) : "");
        Rectangle br = { 34, (float)(296 + row * 44), 272, 36 };
        bool sel = (s_inv_selected == i);
        DrawRectangleRec(br, sel ? (Color){60,60,110,255} : C_BTN);
        DrawRectangleLinesEx(br, 1, C_BORDER);
        txt((int)br.x + 8, (int)br.y + 10, 15, C_WHITE, lbl);
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, br) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            s_inv_selected = (sel ? -1 : i);
            snd_click();
        }
        if (++row >= 10) break;
    }

    Rectangle detail = { 340, 60, SCREEN_W - 360, SCREEN_H - 120 };
    ui_panel(detail, C_PANEL, C_BORDER);

    if (s_inv_selected >= 0 && g_ctx.player.bag_ids[s_inv_selected] >= 0) {
        int id = g_ctx.player.bag_ids[s_inv_selected];
        const ItemDef *it = &g_items[id];
        txt(360, 76, 22, C_GOLD, it->name);
        static const char *type_names[] = { "None","Weapon","Armor","Accessory","Consumable" };
        txt(360, 104, 15, C_DIM, type_names[it->type]);
        txt(360, 124, 15, C_GOLD, TextFormat("Sell value: %d gold", it->price/2));

        int dy = 154;
        if (it->bonus_life) { txt(360, dy, 15, C_HP,    TextFormat("+%d Life",    it->bonus_life)); dy+=22; }
        if (it->bonus_mana) { txt(360, dy, 15, C_MP,    TextFormat("+%d Mana",    it->bonus_mana)); dy+=22; }
        if (it->bonus_str)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Strength",it->bonus_str));  dy+=22; }
        if (it->bonus_spd)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Speed",   it->bonus_spd));  dy+=22; }
        if (it->bonus_def)  { txt(360, dy, 15, C_WHITE, TextFormat("+%d Defense", it->bonus_def));  dy+=22; }

        if (it->type == ITEM_WEAPON || it->type == ITEM_ARMOR || it->type == ITEM_ACCESSORY) {
            Rectangle eb = { 360, (float)(dy + 20), 150, 44 };
            if (ui_button(eb, "Equip", true)) {
                player_equip(&g_ctx.player, id);
                s_inv_selected = -1;
                g_ctx.scene_dirty = true;
            }
        }
        if (it->type == ITEM_CONSUMABLE) {
            Rectangle ub = { 360, (float)(dy + 20), 120, 44 };
            if (ui_button(ub, "Use", true)) {
                player_use_consumable(&g_ctx.player, id);
                s_inv_selected = -1;
            }
        }
        Rectangle sb = { 360, (float)(dy + 74), 120, 44 };
        if (ui_button(sb, "Sell", true)) {
            g_ctx.player.gold += it->price / 2;
            player_remove_item(&g_ctx.player, id);
            s_inv_selected = -1;
            g_ctx.scene_dirty = true;
        }
    } else {
        txt(360, 140, 17, C_DIM, "Select an item from your bag.");
    }

    Rectangle back = { 340, (float)(SCREEN_H - 56), 140, 44 };
    if (ui_button(back, "<- Back", true)) {
        s_inv_selected = -1;
        anim_fade_to(SCENE_HUB);
    }
}
