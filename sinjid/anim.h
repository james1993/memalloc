#pragma once
#include <raylib.h>
#include <stdbool.h>
#include "game.h"

/* Call once per frame before drawing */
void anim_update(float dt);

/* Trigger a scene fade; switches scene at the midpoint */
void anim_fade_to(int scene);

/* Trigger hit flash on player or enemy */
void anim_player_hit(bool heal);
void anim_enemy_hit(bool heal);

/* Reset combat anim to match current player/enemy state */
void anim_combat_reset(void);

/* Y offset for idle bob (pass player_bob_t or enemy_bob_t) */
float anim_bob_y(float t);
