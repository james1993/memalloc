#include "hub.h"
#include "combat.h"
#include "player.h"
#include "data.h"

extern const int g_shop_adv[];
extern const int g_shop_adv_count;

const char *hub_location_name(HubLocation loc)
{
    switch (loc) {
        case HUB_TRAINING:   return "Training Ground";
        case HUB_SHOP_BASIC: return "Beginner's Shop";
        case HUB_SHOP_MID:   return "Mid-Range Store";
        case HUB_SHOP_ADV:   return "Advanced Arsenal";
        case HUB_SKILLS:     return "Skill Master";
        case HUB_INVENTORY:  return "Equipment";
        case HUB_GW_HUMAN:   return "Human Gateway";
        case HUB_GW_MONSTER: return "Monster Portal";
        case HUB_GW_DARK:    return "Dark Rift";
        default:             return "???";
    }
}

bool hub_location_unlocked(HubLocation loc)
{
    switch (loc) {
        case HUB_GW_MONSTER:
            // Unlocked after completing floor 2 of Human Gateway
            return g_player.gw_progress[GW_HUMAN] >= 2;
        case HUB_GW_DARK:
            return g_player.gw_complete[GW_HUMAN];
        case HUB_SHOP_ADV:
            // Advanced shop after level 8
            return g_player.level >= 8;
        default:
            return true;
    }
}

void hub_enter(HubLocation loc)
{
    switch (loc) {
        case HUB_SHOP_BASIC:
            g_shop.item_ids = g_shop_basic;
            g_shop.count    = g_shop_basic_count;
            g_shop.selected = 0;
            g_scene = SCENE_SHOP;
            break;
        case HUB_SHOP_MID:
            g_shop.item_ids = g_shop_mid;
            g_shop.count    = g_shop_mid_count;
            g_shop.selected = 0;
            g_scene = SCENE_SHOP;
            break;
        case HUB_SHOP_ADV:
            g_shop.item_ids = g_shop_adv;
            g_shop.count    = g_shop_adv_count;
            g_shop.selected = 0;
            g_scene = SCENE_SHOP;
            break;
        case HUB_SKILLS:
            g_scene = SCENE_SKILLS;
            break;
        case HUB_INVENTORY:
            g_scene = SCENE_INVENTORY;
            break;
        case HUB_GW_HUMAN:
            g_scene = SCENE_GATEWAY_SELECT;
            // Store gateway id for gateway select screen
            g_combat.gateway = GW_HUMAN;
            break;
        case HUB_GW_MONSTER:
            g_scene = SCENE_GATEWAY_SELECT;
            g_combat.gateway = GW_MONSTER;
            break;
        case HUB_GW_DARK:
            g_scene = SCENE_GATEWAY_SELECT;
            g_combat.gateway = GW_DARK_RIFT;
            break;
        case HUB_TRAINING: {
            // Training: fight a weaker version of the first enemy for XP only
            // Reuse Human Gateway floor 0 but with a training flag
            combat_start(GW_HUMAN, 0);
            // Training doesn't advance gateway progress – handled in apply_rewards
            g_combat.enemy.xp_reward   = 5;
            g_combat.enemy.gold_reward = 0;
            g_scene = SCENE_COMBAT;
            break;
        }
        default:
            break;
    }
}
