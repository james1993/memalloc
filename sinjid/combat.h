#pragma once
#include "game.h"

// Start a new combat against enemy index in g_enemies[]
void combat_start(GatewayId gw, int floor_idx);

// Returns true when combat is over (check g_combat.player_won)
bool combat_is_over(void);

// Player actions – each returns false if the action is invalid right now
bool combat_action_attack(void);
bool combat_action_skill(int skill_idx);
bool combat_action_use_item(int item_id);
bool combat_action_flee(void);

// Advance combat by one enemy turn (call after player's turn ends)
void combat_enemy_turn(void);

// Append a line to the combat log
void combat_log(const char *fmt, ...);

// Called when combat ends (victory or defeat) to hand out rewards
void combat_apply_rewards(void);
