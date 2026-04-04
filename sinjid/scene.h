#pragma once
#include <stdbool.h>

typedef enum {
    SCENE_TITLE,
    SCENE_CLASS_SELECT,
    SCENE_HUB,
    SCENE_COMBAT,
    SCENE_INVENTORY,
    SCENE_SHOP,
    SCENE_SKILLS,
    SCENE_LEVEL_UP,
    SCENE_GATEWAY_SELECT,
    SCENE_GAME_OVER,
    SCENE_VICTORY,
    SCENE_COUNT
} Scene;

#define SCENE_STACK_CAP 8

typedef struct {
    Scene stack[SCENE_STACK_CAP];
    int   top;   // index of current scene (-1 = empty)
} SceneStack;

void  scene_init(SceneStack *ss, Scene initial);
Scene scene_current(const SceneStack *ss);
void  scene_push(SceneStack *ss, Scene s);
Scene scene_pop(SceneStack *ss);       // returns popped scene
bool  scene_can_pop(const SceneStack *ss);
void  scene_replace(SceneStack *ss, Scene s); // pop + push (no history)
