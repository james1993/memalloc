#pragma once

// Initialize and close the audio device
void audio_init(void);
void audio_close(void);

// Sound events
void snd_click(void);       // UI button press
void snd_hover(void);       // Button hover
void snd_hit(void);         // Physical attack lands
void snd_skill(void);       // Skill activation
void snd_miss(void);        // Dodge / miss
void snd_poison(void);      // Poison tick
void snd_level_up(void);    // Level up fanfare
void snd_victory(void);     // Combat victory
void snd_game_over(void);   // Defeat
