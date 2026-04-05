#include "scene.h"
#include <raylib.h>

void scene_init(SceneStack *ss, Scene initial)
{
    ss->top      = 0;
    ss->stack[0] = initial;
}

Scene scene_current(const SceneStack *ss)
{
    return ss->stack[ss->top];
}

void scene_push(SceneStack *ss, Scene s)
{
    if (ss->top >= SCENE_STACK_CAP - 1) {
        TraceLog(LOG_ERROR, "SCENE: stack overflow — push ignored (scene %d)", (int)s);
        return;
    }
    ss->stack[++ss->top] = s;
}

Scene scene_pop(SceneStack *ss)
{
    if (ss->top <= 0) return ss->stack[0];
    Scene popped = ss->stack[ss->top--];
    return popped;
}

bool scene_can_pop(const SceneStack *ss)
{
    return ss->top > 0;
}

void scene_replace(SceneStack *ss, Scene s)
{
    ss->stack[ss->top] = s;
}
