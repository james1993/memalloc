#pragma once
#include "game.h"
#include <stdbool.h>

#define SAVE_VERSION  3          // bump when save format changes
#define SAVE_PATH     "sinjid_save.dat"

// Returns true on success
bool save_game(const Player *p);
bool load_game(Player *p);

// Returns true if a save file exists
bool save_exists(void);

// Deletes the save file
void save_delete(void);
