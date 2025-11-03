#ifndef FORCEFIELD_H
#define FORCEFIELD_H

#include "raylib.h"
#include "config.h"

typedef enum {
    FF_STATE_READY,
    FF_STATE_ACTIVE,
    FF_STATE_COOLDOWN
} ForceFieldState;

extern ForceFieldState ff_state;
extern float ff_charge;

void InitForceField(void);
void UpdateForceField(Sound forceFieldHitSound);
void ActivateForceField(Sound forceFieldSound, Sound forceFailSound);
void DrawForceField2D(void);
void DrawForceFieldUI(void);

#endif // FORCEFIELD_H
