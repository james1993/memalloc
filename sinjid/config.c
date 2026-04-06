#include "config.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

Config g_config;

void config_init_defaults(void)
{
    g_config.window_w = 1280;
    g_config.window_h = 960;
    g_config.fps      = 60;
    g_config.volume   = 1.0f;
    strncpy(g_config.save_path, "sinjid_save.dat", sizeof(g_config.save_path) - 1);
    strncpy(g_config.data_dir,  "data",            sizeof(g_config.data_dir)  - 1);
}

static char *ltrim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    return s;
}

int config_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *l = ltrim(line);
        if (!*l || *l == '#' || *l == ';') continue;

        char *eq = strchr(l, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = l;
        char *val = ltrim(eq + 1);
        /* strip trailing whitespace/newline */
        int vlen = (int)strlen(val);
        while (vlen > 0 && isspace((unsigned char)val[vlen-1])) val[--vlen] = '\0';

        if      (strcmp(key, "window_w")  == 0) g_config.window_w = atoi(val);
        else if (strcmp(key, "window_h")  == 0) g_config.window_h = atoi(val);
        else if (strcmp(key, "fps")       == 0) g_config.fps      = atoi(val);
        else if (strcmp(key, "volume")    == 0) g_config.volume   = (float)atof(val);
        else if (strcmp(key, "save_path") == 0) {
            if (strlen(val) >= sizeof(g_config.save_path))
                TraceLog(LOG_WARNING, "CONFIG: save_path value truncated to %d chars",
                         (int)(sizeof(g_config.save_path) - 1));
            strncpy(g_config.save_path, val, sizeof(g_config.save_path)-1);
        }
        else if (strcmp(key, "data_dir")  == 0) {
            if (strlen(val) >= sizeof(g_config.data_dir))
                TraceLog(LOG_WARNING, "CONFIG: data_dir value truncated to %d chars",
                         (int)(sizeof(g_config.data_dir) - 1));
            strncpy(g_config.data_dir,  val, sizeof(g_config.data_dir) -1);
        }
    }
    fclose(f);
    return 1;
}

int config_save(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f,
        "# Sinjid configuration\n"
        "window_w  = %d\n"
        "window_h  = %d\n"
        "fps       = %d\n"
        "volume    = %.2f\n"
        "save_path = %s\n"
        "data_dir  = %s\n",
        g_config.window_w, g_config.window_h, g_config.fps,
        g_config.volume, g_config.save_path, g_config.data_dir);
    fclose(f);
    return 1;
}
