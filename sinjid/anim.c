#include "anim.h"
#include "game.h"
#include <math.h>

CombatAnim g_canim;
FadeAnim   g_fade;
bool       g_scene_dirty = true;

#define LERP_SPEED   4.0f   // HP bar lerp rate (units/sec proportional)
#define FLASH_DECAY  3.5f   // flash fade rate per second
#define BOB_SPEED    1.2f   // idle bob oscillation Hz
#define BOB_AMP      3.0f   // pixels of vertical bob
#define FADE_SPEED   2.2f   // scene fade alpha per second

void anim_combat_reset(void)
{
    g_canim.player_life = (float)g_player.current_life;
    g_canim.player_mana = (float)g_player.current_mana;
    g_canim.enemy_life  = (float)g_combat.enemy.current_life;
    g_canim.enemy_mana  = (float)g_combat.enemy.current_mana;
    g_canim.player_flash = 0.0f;
    g_canim.enemy_flash  = 0.0f;
    g_canim.player_bob_t = 0.0f;
    g_canim.enemy_bob_t  = 0.3f; // offset phase so they don't sync
}

void anim_player_hit(bool heal)
{
    g_canim.player_flash     = 1.0f;
    g_canim.player_flash_col = heal;
}

void anim_enemy_hit(bool heal)
{
    g_canim.enemy_flash     = 1.0f;
    g_canim.enemy_flash_col = heal;
}

void anim_fade_to(int scene)
{
    if (g_fade.active) return;
    g_fade.target_scene = scene;
    g_fade.fading_out   = true;
    g_fade.active       = true;
}

float anim_bob_y(float t)
{
    return sinf(2.0f * PI * BOB_SPEED * t) * BOB_AMP;
}

static float lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

void anim_update(float dt)
{
    /* ── Idle bob ── */
    g_canim.player_bob_t += dt;
    g_canim.enemy_bob_t  += dt;
    /* Wrap to avoid float drift after long sessions */
    if (g_canim.player_bob_t > 1000.0f) g_canim.player_bob_t -= 1000.0f;
    if (g_canim.enemy_bob_t  > 1000.0f) g_canim.enemy_bob_t  -= 1000.0f;

    /* ── HP/MP bar lerp ── */
    float lspeed = LERP_SPEED * dt;
    if (lspeed > 1.0f) lspeed = 1.0f;

    float tpl = (float)g_player.current_life;
    float tpm = (float)g_player.current_mana;
    float tel = (float)g_combat.enemy.current_life;
    float tem = (float)g_combat.enemy.current_mana;

    g_canim.player_life = lerpf(g_canim.player_life, tpl, lspeed);
    g_canim.player_mana = lerpf(g_canim.player_mana, tpm, lspeed);
    g_canim.enemy_life  = lerpf(g_canim.enemy_life,  tel, lspeed);
    g_canim.enemy_mana  = lerpf(g_canim.enemy_mana,  tem, lspeed);

    /* ── Hit flash decay ── */
    g_canim.player_flash -= FLASH_DECAY * dt;
    g_canim.enemy_flash  -= FLASH_DECAY * dt;
    if (g_canim.player_flash < 0.0f) g_canim.player_flash = 0.0f;
    if (g_canim.enemy_flash  < 0.0f) g_canim.enemy_flash  = 0.0f;

    /* ── Scene fade ── */
    if (g_fade.active) {
        if (g_fade.fading_out) {
            g_fade.fade_alpha += FADE_SPEED * dt;
            if (g_fade.fade_alpha >= 1.0f) {
                g_fade.fade_alpha = 1.0f;
                g_fade.fading_out = false;
                g_scene = (Scene)g_fade.target_scene;
                g_scene_dirty = true;
            }
        } else {
            g_fade.fade_alpha -= FADE_SPEED * dt;
            if (g_fade.fade_alpha <= 0.0f) {
                g_fade.fade_alpha = 0.0f;
                g_fade.active     = false;
            }
        }
    }
}
