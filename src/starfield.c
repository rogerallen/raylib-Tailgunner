//================================================================================================
//
//   starfield.c - Background starfield effect implementation
//
//   See starfield.h for module interface documentation.
//
//   Implementation notes:
//   - Uses instanced rendering to draw all stars efficiently
//   - Star positions are updated on the CPU each frame
//   - Custom shader handles per-instance transformation via matModel uniform
//
//================================================================================================

#include "starfield.h"
#include "gl_debug.h"
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
    starfield.material.shader = LoadShader(TextFormat("resources/shaders/glsl%i/starfield.vs", GLSL_VERSION),
                                           TextFormat("resources/shaders/glsl%i/starfield.fs", GLSL_VERSION));

    // Get shader uniform locations for transformation matrices
    // following line is a key! difference from example code.  Found on reddit.
    starfield.material.shader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocationAttrib(starfield.material.shader, "instanceTransform");
    starfield.material.shader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(starfield.material.shader, "matView");
    starfield.material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] =
        GetShaderLocation(starfield.material.shader, "matProjection");

    starfield.transforms = (Matrix *)RL_MALLOC(MAX_STARS * sizeof(Matrix));
    starfield.positions = (Vector3 *)RL_MALLOC(MAX_STARS * sizeof(Vector3));

    for (int i = 0; i < MAX_STARS; i++) {
        starfield.positions[i] =
            (Vector3){GetRandomValue(-100, 100), GetRandomValue(-100, 100), GetRandomValue(-200, 0)};
        starfield.transforms[i] =
            MatrixTranslate(starfield.positions[i].x, starfield.positions[i].y, starfield.positions[i].z);
    }
}

void UnloadStarfield(void)
{
    UnloadMesh(starfield.mesh);
    UnloadMaterial(starfield.material);
    RL_FREE(starfield.transforms);
    RL_FREE(starfield.positions);
}

void UpdateStarfield(void)
{
    const float speed = 60.0f;
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_STARS; i++) {
        starfield.positions[i].z -= speed * dt;
        if (starfield.positions[i].z < -200.0f) {
            starfield.positions[i].x = GetRandomValue(-100, 100);
            starfield.positions[i].y = GetRandomValue(-100, 100);
            starfield.positions[i].z = 0.0f;
        }
        starfield.transforms[i] =
            MatrixTranslate(starfield.positions[i].x, starfield.positions[i].y, starfield.positions[i].z);
    }
}

void DrawStarfield(void)
{
    // Draw all the stars via instancing
    DrawMeshInstanced(starfield.mesh, starfield.material, starfield.transforms, MAX_STARS);
}
