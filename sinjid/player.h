#pragma once
#include "game.h"

// Initialize a fresh player with a chosen class
void player_init(Player *p, const char *name, PlayerClass pc);

// Compute total stat including equipment bonuses
int  player_effective_str(void);
int  player_effective_spd(void);
int  player_effective_def(void);
int  player_effective_life(void);
int  player_effective_mana(void);

// Add XP; returns true if levelled up
bool player_add_xp(Player *p, int xp);

// Apply XP-to-next formula for a given level
int  xp_for_level(int level);

// Add item to bag; returns false if bag full
bool player_add_item(Player *p, int item_id);

// Remove one of item_id from bag; returns false if not found
bool player_remove_item(Player *p, int item_id);

// Equip item from bag into correct slot; unequips old item back to bag
void player_equip(Player *p, int item_id);

// Unequip slot back into bag
void player_unequip(Player *p, EquipSlot slot);

// Apply consumable; returns false if item is not consumable / not in bag
bool player_use_consumable(Player *p, int item_id);

// Restore life/mana fully (between fights)
void player_rest(Player *p);

// Tick status (called at start of player turn): returns damage taken
int  player_tick_status(Player *p);

// Returns display name for a class
const char *class_name(PlayerClass pc);

// Returns starting stats for a class
Stats class_base_stats(PlayerClass pc);
