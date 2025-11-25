//================================================================================================
//
//   starfield.c - Background starfield effect implementation
//
//   See starfield.h for module interface documentation.
//
//   Implementation notes:
//   - Uses instanced rendering to draw all stars in a single draw call.
//   - Star positions are updated on the CPU.
//   - The transformation matrices are re-uploaded to the GPU each frame via DrawMeshInstanced.
//
//================================================================================================

#include "starfield.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdlib.h>

//----------------------------------------------------------------------------------
// Module Variables
//----------------------------------------------------------------------------------
Starfield starfield;

//----------------------------------------------------------------------------------
// Public Function Implementations (see starfield.h for documentation)
//----------------------------------------------------------------------------------

void InitStarfield(void)
{
    starfield.mesh = GenMeshCube(0.1f, 0.1f, 0.1f);
    starfield.material = LoadMaterialDefault();
    starfield.material.shader = LoadShader("src/starfield.vs", "src/starfield.fs");
    starfield.transforms = (Matrix *)RL_MALLOC(MAX_STARS * sizeof(Matrix));
    starfield.positions = (Vector3 *)RL_MALLOC(MAX_STARS * sizeof(Vector3));

    for (int i = 0; i < MAX_STARS; i++)
    {
        starfield.positions[i] = (Vector3){GetRandomValue(-100, 100), GetRandomValue(-100, 100), GetRandomValue(0, 200)};
        starfield.transforms[i] = MatrixTranslate(starfield.positions[i].x, starfield.positions[i].y, starfield.positions[i].z);
    }
}

void UpdateStarfield(void)
{
    const float speed = 60.0f;
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_STARS; i++)
    {
        starfield.positions[i].z -= speed * dt;
        if (starfield.positions[i].z < 0)
        {
            starfield.positions[i].x = GetRandomValue(-100, 100);
            starfield.positions[i].y = GetRandomValue(-100, 100);
            starfield.positions[i].z = 200;
        }
        starfield.transforms[i] = MatrixTranslate(starfield.positions[i].x, starfield.positions[i].y, starfield.positions[i].z);
    }
}

void DrawStarfield(void)
{
    DrawMeshInstanced(starfield.mesh, starfield.material, starfield.transforms, MAX_STARS);
}
