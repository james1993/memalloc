#pragma once

#define CONFIG_PATH "config.ini"

typedef struct {
    int   window_w;
    int   window_h;
    int   fps;
    float volume;       /* 0.0 – 1.0 master volume */
    char  save_path[256];
    char  data_dir[256];
} Config;

/* Single global config instance – defined in config.c */
extern Config g_config;

/* Fill g_config with built-in defaults */
void config_init_defaults(void);

/* Load key=value pairs from path; unknown keys are silently ignored.
   Returns 1 on success, 0 if file not found (defaults remain). */
int config_load(const char *path);

/* Write current g_config back to path */
int config_save(const char *path);
