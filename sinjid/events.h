#pragma once
#include <stdbool.h>

typedef enum {
    EVT_NONE = 0,
    EVT_HIT_ENEMY,       // player damaged enemy   (value = damage)
    EVT_HIT_PLAYER,      // enemy damaged player   (value = damage)
    EVT_HEAL_PLAYER,     // player healed          (value = amount)
    EVT_SKILL_USE,       // player used a skill
    EVT_POISON_TICK,     // poison dealt damage    (value = damage)
    EVT_STUN,            // target stunned
    EVT_COMBAT_WIN,      // combat ended in victory
    EVT_COMBAT_LOSE,     // combat ended in defeat
    EVT_LEVEL_UP,        // player gained a level
    EVT_FLEE,            // player fled combat
    EVT_UI_CLICK,        // button clicked
    EVT_UI_HOVER,        // button hovered
    EVT_SCENE_CHANGE,    // scene transition triggered
} EventType;

typedef struct {
    EventType type;
    int       value;     // damage / heal / level / 0
    bool      for_player; // true = affects player, false = enemy
} Event;

#define EVT_QUEUE_CAP 64

typedef struct {
    Event events[EVT_QUEUE_CAP];
    int   head;
    int   tail;
    int   count;
} EventQueue;

void  evt_init(EventQueue *q);
void  evt_push(EventQueue *q, EventType type, int value, bool for_player);
bool  evt_pop(EventQueue *q, Event *out);
bool  evt_peek(const EventQueue *q, Event *out);
void  evt_clear(EventQueue *q);
