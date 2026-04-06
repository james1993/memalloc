#pragma once

/* Advance all game-logic state for the current frame.
   Call this BEFORE draw_scene() in the main loop. */
void scene_update(float dt);
