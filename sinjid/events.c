#include "events.h"
#include <string.h>

void evt_init(EventQueue *q)
{
    memset(q, 0, sizeof(*q));
}

void evt_push(EventQueue *q, EventType type, int value, bool for_player)
{
    if (q->count >= EVT_QUEUE_CAP) return; // drop if full
    q->events[q->tail].type       = type;
    q->events[q->tail].value      = value;
    q->events[q->tail].for_player = for_player;
    q->tail  = (q->tail + 1) % EVT_QUEUE_CAP;
    q->count++;
}

bool evt_pop(EventQueue *q, Event *out)
{
    if (q->count == 0) return false;
    *out     = q->events[q->head];
    q->head  = (q->head + 1) % EVT_QUEUE_CAP;
    q->count--;
    return true;
}

bool evt_peek(const EventQueue *q, Event *out)
{
    if (q->count == 0) return false;
    *out = q->events[q->head];
    return true;
}

void evt_clear(EventQueue *q)
{
    q->head = q->tail = q->count = 0;
}
