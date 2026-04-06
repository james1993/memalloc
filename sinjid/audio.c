#include "audio.h"
#include "config.h"
#include <raylib.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE  44100
#define MAX_FRAMES   (SAMPLE_RATE * 2)

/* ── Wave synthesis helpers ── */

typedef struct { float freq; float dur; float vol; } Note;

/* Generate a single sine tone with an ADSR-like envelope into a float buffer.
   buf must have at least (int)(dur*SAMPLE_RATE) floats.
   Returns number of frames written. */
static int gen_sine(float *buf, float freq, float dur, float vol,
                    float attack, float release)
{
    int n = (int)(dur * SAMPLE_RATE);
    int na = (int)(attack  * SAMPLE_RATE);
    int nr = (int)(release * SAMPLE_RATE);
    for (int i = 0; i < n; i++) {
        float t   = (float)i / SAMPLE_RATE;
        float env = 1.0f;
        if (i < na) env = (float)i / na;
        else if (i >= n - nr) env = (float)(n - i) / nr;
        buf[i] += vol * env * sinf(2.0f * PI * freq * t);
    }
    return n;
}

/* Square wave (for retro blips) */
static int gen_square(float *buf, float freq, float dur, float vol,
                      float attack, float release)
{
    int n = (int)(dur * SAMPLE_RATE);
    int na = (int)(attack  * SAMPLE_RATE);
    int nr = (int)(release * SAMPLE_RATE);
    for (int i = 0; i < n; i++) {
        float t   = (float)i / SAMPLE_RATE;
        float env = 1.0f;
        if (i < na) env = (float)i / na;
        else if (i >= n - nr) env = (float)(n - i) / nr;
        float wave = sinf(2.0f * PI * freq * t) >= 0.0f ? 1.0f : -1.0f;
        buf[i] += vol * env * wave;
    }
    return n;
}

/* White noise burst */
static int gen_noise(float *buf, float dur, float vol, float release)
{
    int n  = (int)(dur * SAMPLE_RATE);
    int nr = (int)(release * SAMPLE_RATE);
    for (int i = 0; i < n; i++) {
        float env = 1.0f;
        if (i >= n - nr) env = (float)(n - i) / nr;
        buf[i] += vol * env * ((float)rand() / RAND_MAX * 2.0f - 1.0f);
    }
    return n;
}

/* Build a Sound from a float buffer (normalized to 16-bit PCM) */
static Sound make_sound(float *buf, int frames)
{
    /* Normalize */
    float peak = 0.001f;
    for (int i = 0; i < frames; i++)
        if (fabsf(buf[i]) > peak) peak = fabsf(buf[i]);

    short *pcm = malloc(frames * sizeof(short));
    for (int i = 0; i < frames; i++)
        pcm[i] = (short)(buf[i] / peak * 30000.0f);

    Wave w = {
        .frameCount = (unsigned)frames,
        .sampleRate = SAMPLE_RATE,
        .sampleSize = 16,
        .channels   = 1,
        .data       = pcm
    };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);  // frees w.data (pcm) through Raylib's allocator
    return s;
}

/* ── Sound library ── */
static Sound s_click;
static Sound s_hover;
static Sound s_hit;
static Sound s_skill;
static Sound s_miss;
static Sound s_poison;
static Sound s_level_up;
static Sound s_victory;
static Sound s_game_over;

static float work[MAX_FRAMES];

#define WIPE() memset(work, 0, sizeof(work))

void audio_init(void)
{
    InitAudioDevice();
    SetMasterVolume(g_config.volume);

    /* Click – short square blip at 600Hz */
    WIPE();
    gen_square(work, 600.0f, 0.04f, 0.5f, 0.002f, 0.02f);
    s_click = make_sound(work, (int)(0.04f * SAMPLE_RATE));

    /* Hover – softer blip at 500Hz */
    WIPE();
    gen_square(work, 500.0f, 0.025f, 0.2f, 0.002f, 0.015f);
    s_hover = make_sound(work, (int)(0.025f * SAMPLE_RATE));

    /* Hit – low thud: mixed 80Hz + noise */
    WIPE();
    gen_sine(work, 80.0f, 0.18f, 0.6f, 0.003f, 0.12f);
    gen_noise(work, 0.10f, 0.3f, 0.08f);
    s_hit = make_sound(work, (int)(0.18f * SAMPLE_RATE));

    /* Skill – rising chirp: sweep 300→900Hz via two partials */
    WIPE();
    gen_sine(work, 350.0f, 0.15f, 0.5f, 0.01f, 0.08f);
    gen_sine(work, 700.0f, 0.15f, 0.3f, 0.04f, 0.06f);
    gen_sine(work,1050.0f, 0.15f, 0.2f, 0.08f, 0.04f);
    s_skill = make_sound(work, (int)(0.15f * SAMPLE_RATE));

    /* Miss – short descending whistle */
    WIPE();
    gen_sine(work, 900.0f, 0.06f, 0.3f, 0.003f, 0.04f);
    gen_sine(work, 600.0f, 0.06f, 0.3f, 0.02f,  0.03f);
    s_miss = make_sound(work, (int)(0.09f * SAMPLE_RATE));

    /* Poison – gurgly mid buzz */
    WIPE();
    gen_square(work, 180.0f, 0.12f, 0.3f, 0.01f, 0.06f);
    gen_noise(work,  0.12f, 0.1f, 0.06f);
    s_poison = make_sound(work, (int)(0.12f * SAMPLE_RATE));

    /* Level up – C-E-G-C arpeggio */
    {
        float notes[] = { 261.6f, 329.6f, 392.0f, 523.3f };
        float total   = 4 * 0.12f + 0.15f;
        int   ntotal  = (int)(total * SAMPLE_RATE);
        WIPE();
        for (int i = 0; i < 4; i++) {
            float *p = work + (int)(i * 0.12f * SAMPLE_RATE);
            gen_sine(p, notes[i], 0.20f, 0.5f, 0.005f, 0.12f);
        }
        s_level_up = make_sound(work, ntotal);
    }

    /* Victory – triumphant G-B-D-G-B chord sweep */
    {
        float notes[] = { 392.0f, 493.9f, 587.3f, 784.0f, 987.8f };
        int   ntotal  = (int)(0.8f * SAMPLE_RATE);
        WIPE();
        for (int i = 0; i < 5; i++) {
            float *p = work + (int)(i * 0.10f * SAMPLE_RATE);
            gen_sine(p, notes[i], 0.55f, 0.4f, 0.005f, 0.30f);
        }
        s_victory = make_sound(work, ntotal);
    }

    /* Game over – descending minor: A-F-D-A */
    {
        float notes[] = { 440.0f, 349.2f, 293.7f, 220.0f };
        int   ntotal  = (int)(1.4f * SAMPLE_RATE);
        WIPE();
        for (int i = 0; i < 4; i++) {
            float *p = work + (int)(i * 0.30f * SAMPLE_RATE);
            gen_sine(p, notes[i], 0.45f, 0.5f, 0.01f, 0.25f);
        }
        s_game_over = make_sound(work, ntotal);
    }
}

void audio_close(void)
{
    UnloadSound(s_click);
    UnloadSound(s_hover);
    UnloadSound(s_hit);
    UnloadSound(s_skill);
    UnloadSound(s_miss);
    UnloadSound(s_poison);
    UnloadSound(s_level_up);
    UnloadSound(s_victory);
    UnloadSound(s_game_over);
    CloseAudioDevice();
}

void snd_click(void)    { PlaySound(s_click);    }
void snd_hover(void)    { PlaySound(s_hover);     }
void snd_hit(void)      { PlaySound(s_hit);       }
void snd_skill(void)    { PlaySound(s_skill);     }
void snd_miss(void)     { PlaySound(s_miss);      }
void snd_poison(void)   { PlaySound(s_poison);    }
void snd_level_up(void) { PlaySound(s_level_up);  }
void snd_victory(void)  { PlaySound(s_victory);   }
void snd_game_over(void){ PlaySound(s_game_over); }
