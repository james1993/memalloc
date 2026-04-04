#pragma once
#include "game.h"

/* Consumable heal effect (used by player.c) */
typedef struct { int life; int mana; } ConsumeEffect;
extern ConsumeEffect g_consume_effect[];
extern const int     g_consume_effect_count;

/* Initialise all data tables from built-in defaults.
   Call once at startup before data_load_files(). */
void data_init_defaults(void);

/* Load data from .dat files in the given directory.
   Silently ignores missing files (defaults remain).
   Returns the number of files successfully loaded. */
int data_load_files(const char *dir);

/* Skill getter */
const SkillDef *skills_for_class(PlayerClass pc);
