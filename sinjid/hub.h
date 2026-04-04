#pragma once
#include "game.h"

// Hub locations the player can visit
typedef enum {
    HUB_NONE = -1,
    HUB_TRAINING,
    HUB_SHOP_BASIC,
    HUB_SHOP_MID,
    HUB_SHOP_ADV,
    HUB_SKILLS,
    HUB_INVENTORY,
    HUB_GW_HUMAN,
    HUB_GW_MONSTER,
    HUB_GW_DARK,
    HUB_LOC_COUNT
} HubLocation;

// Enter a hub location; triggers scene transitions as needed
void hub_enter(HubLocation loc);

// Display name for a hub location
const char *hub_location_name(HubLocation loc);

// Whether a hub location is currently accessible
bool hub_location_unlocked(HubLocation loc);
