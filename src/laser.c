/*******************************************************************************************
*
*   laser.c - Laser weapon system implementation
*
*   See laser.h for module interface documentation.
*   
*   Implementation notes:
*   - Uses ray-sphere intersection for enemy hit detection
*   - Beams start slightly offset (left/right) from camera for visual effect
*   - Overwrites oldest beam if all slots are active when firing
*
*******************************************************************************************/

#include "laser.h"
#include "enemy.h"
#include "raymath.h"
#include "config.h"

//----------------------------------------------------------------------------------
// Module Variables
//----------------------------------------------------------------------------------

// No global lasers array. Storage is held in LaserManager provided by caller.

//----------------------------------------------------------------------------------
// Public Function Implementations (see laser.h for documentation)
//----------------------------------------------------------------------------------

void InitLasers(LaserManager* mgr)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        mgr->lasers[i].active = false;
        mgr->lasers[i].lifeTime = 0.0f;
        mgr->lasers[i].color = RED;
    }
}

//----------------------------------------------------------------------------------
// FireLasers - Implementation Notes:
// - Uses ray-sphere intersection test to detect enemy hits
// - Places beam start points offset from camera for visual effect
// - Only destroys closest enemy hit by ray
// - Will overwrite oldest beam if all slots are active
//----------------------------------------------------------------------------------
int FireLasers(LaserManager* lmgr, struct EnemyManager* emgr, Ray ray, Camera camera, Sound explosionSound)
{
    int hits = 0;
    // Find closest enemy hit by the ray
    float closestHitDist = 1e6f;
    int closestEnemyIndex = -1;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (emgr->enemies[i].active)
        {
            RayCollision collision = GetRayCollisionSphere(ray, emgr->enemies[i].position, emgr->enemies[i].radius * 1.5f);
            if (collision.hit && collision.distance < closestHitDist)
            {
                closestHitDist = collision.distance;
                closestEnemyIndex = i;
            }
        }
    }

    // Default end position far along the ray
    Vector3 endPos = Vector3Add(camera.position, Vector3Scale(ray.direction, 1000.0f));
    // Compute camera basis to place beam starts relative to camera orientation
    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camUp = camera.up;
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, camUp));

    if (closestEnemyIndex != -1)
    {
        endPos = emgr->enemies[closestEnemyIndex].position;
        emgr->enemies[closestEnemyIndex].active = false;
        hits++;
        // Play explosion sound here is OK, but caller may prefer to handle audio; keep for now
        PlaySound(explosionSound);
    }

    // Spawn beam(s) into available laser slots. Place starts slightly offset left/right and forward.
    int placed = 0;
    for (int i = 0; i < MAX_LASERS && placed < MAX_LASERS; i++)
    {
        if (!lmgr->lasers[i].active)
        {
            // Alternate left/right offset per laser slot
            float horiz = (i % 2 == 0) ? -0.5f : 0.5f;
            Vector3 offset = Vector3Add(Vector3Scale(camRight, horiz), Vector3Scale(camUp, -0.5f));
                Vector3 startPos = Vector3Add(camera.position, Vector3Add(Vector3Scale(camForward, LASER_START_FORWARD_OFFSET), offset));

            lmgr->lasers[i].active = true;
            lmgr->lasers[i].lifeTime = LASER_LIFETIME; // use configurable lifetime
            lmgr->lasers[i].start = startPos;
            lmgr->lasers[i].end = endPos;
            lmgr->lasers[i].color = RED;
            placed++;
        }
    }

    // If we couldn't place any (all slots busy), overwrite the oldest slot (index 0) so the shot is visible
    if (placed == 0 && MAX_LASERS > 0)
    {
        float horiz = 0.0f;
        Vector3 offset = Vector3Add(Vector3Scale(camRight, horiz), Vector3Scale(camUp, -0.5f));
            Vector3 startPos = Vector3Add(camera.position, Vector3Add(Vector3Scale(camForward, LASER_START_FORWARD_OFFSET), offset));

        lmgr->lasers[0].active = true;
        lmgr->lasers[0].lifeTime = LASER_LIFETIME;
        lmgr->lasers[0].start = startPos;
        lmgr->lasers[0].end = endPos;
        lmgr->lasers[0].color = RED;
    }

    return hits;
}

//----------------------------------------------------------------------------------
// UpdateLasers - Implementation Notes:
// - Decrements lifetime of active beams using frame time
// - Deactivates beams when lifetime expires
//----------------------------------------------------------------------------------
void UpdateLasers(LaserManager* mgr)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (mgr->lasers[i].active)
        { 
            mgr->lasers[i].lifeTime -= GetFrameTime();
            if (mgr->lasers[i].lifeTime <= 0.0f)
            {
                mgr->lasers[i].active = false;
            }
        }
    }
}

//----------------------------------------------------------------------------------
// DrawLasers - Implementation Notes:
// - Renders active laser beams as 3D lines
// - Uses beam color property for rendering
//----------------------------------------------------------------------------------
void DrawLasers(LaserManager* mgr)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (mgr->lasers[i].active)
        {
            DrawLine3D(mgr->lasers[i].start, mgr->lasers[i].end, mgr->lasers[i].color);
        }
    }
}
