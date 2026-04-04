#include "save.h"
#include <stdio.h>
#include <string.h>

/*
 * Save format (binary, little-endian):
 *   uint32  magic        = 0x534E4A44  ("SNJD")
 *   uint32  version      = SAVE_VERSION
 *   Player  player       (raw struct dump)
 *   uint32  checksum     (simple sum of all Player bytes mod 2^32)
 *
 * The Player struct contains no pointers, so a raw dump is safe.
 * If the version or magic doesn't match we refuse to load.
 */

#define MAGIC 0x534E4A44u

static unsigned int checksum(const Player *p)
{
    const unsigned char *b = (const unsigned char *)p;
    unsigned int sum = 0;
    for (size_t i = 0; i < sizeof(Player); i++)
        sum += b[i];
    return sum;
}

bool save_game(const Player *p)
{
    FILE *f = fopen(SAVE_PATH, "wb");
    if (!f) return false;

    unsigned int magic   = MAGIC;
    unsigned int version = SAVE_VERSION;
    unsigned int csum    = checksum(p);

    fwrite(&magic,   sizeof(magic),   1, f);
    fwrite(&version, sizeof(version), 1, f);
    fwrite(p,        sizeof(Player),  1, f);
    fwrite(&csum,    sizeof(csum),    1, f);

    fclose(f);
    return true;
}

bool load_game(Player *p)
{
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f) return false;

    unsigned int magic   = 0;
    unsigned int version = 0;
    Player       tmp;
    unsigned int csum    = 0;

    bool ok =
        fread(&magic,   sizeof(magic),   1, f) == 1 &&
        fread(&version, sizeof(version), 1, f) == 1 &&
        fread(&tmp,     sizeof(Player),  1, f) == 1 &&
        fread(&csum,    sizeof(csum),    1, f) == 1;

    fclose(f);

    if (!ok)                          return false;
    if (magic   != MAGIC)             return false;
    if (version != SAVE_VERSION)      return false;
    if (csum    != checksum(&tmp))    return false;

    memcpy(p, &tmp, sizeof(Player));
    return true;
}

bool save_exists(void)
{
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void save_delete(void)
{
    remove(SAVE_PATH);
}
