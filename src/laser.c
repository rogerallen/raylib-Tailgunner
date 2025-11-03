#include "laser.h"
#include "enemy.h"
#include "raymath.h"
#include "config.h"

Laser lasers[MAX_LASERS];

void InitLasers(void)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        lasers[i].active = false;
        lasers[i].lifeTime = 0.0f;
        lasers[i].color = RED;
    }
}

int FireLasers(Ray ray, Camera camera, Sound explosionSound)
{
    int hits = 0;
    // Find closest enemy hit by the ray
    float closestHitDist = 1e6f;
    int closestEnemyIndex = -1;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            RayCollision collision = GetRayCollisionSphere(ray, enemies[i].position, enemies[i].radius * 1.5f);
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
        endPos = enemies[closestEnemyIndex].position;
        enemies[closestEnemyIndex].active = false;
        hits++;
        // Play explosion sound here is OK, but caller may prefer to handle audio; keep for now
        PlaySound(explosionSound);
    }

    // Spawn beam(s) into available laser slots. Place starts slightly offset left/right and forward.
    int placed = 0;
    for (int i = 0; i < MAX_LASERS && placed < MAX_LASERS; i++)
    {
        if (!lasers[i].active)
        {
            // Alternate left/right offset per laser slot
            float horiz = (i % 2 == 0) ? -0.5f : 0.5f;
            Vector3 offset = Vector3Add(Vector3Scale(camRight, horiz), Vector3Scale(camUp, -0.5f));
                Vector3 startPos = Vector3Add(camera.position, Vector3Add(Vector3Scale(camForward, LASER_START_FORWARD_OFFSET), offset));

            lasers[i].active = true;
                lasers[i].lifeTime = LASER_LIFETIME; // use configurable lifetime
            lasers[i].start = startPos;
            lasers[i].end = endPos;
            lasers[i].color = RED;
            placed++;
        }
    }

    // If we couldn't place any (all slots busy), overwrite the oldest slot (index 0) so the shot is visible
    if (placed == 0 && MAX_LASERS > 0)
    {
        float horiz = 0.0f;
        Vector3 offset = Vector3Add(Vector3Scale(camRight, horiz), Vector3Scale(camUp, -0.5f));
            Vector3 startPos = Vector3Add(camera.position, Vector3Add(Vector3Scale(camForward, LASER_START_FORWARD_OFFSET), offset));

        lasers[0].active = true;
            lasers[0].lifeTime = LASER_LIFETIME;
        lasers[0].start = startPos;
        lasers[0].end = endPos;
        lasers[0].color = RED;
    }

    return hits;
}

void UpdateLasers(void)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (lasers[i].active)
        { 
            lasers[i].lifeTime -= GetFrameTime();
            if (lasers[i].lifeTime <= 0.0f)
            {
                lasers[i].active = false;
            }
        }
    }
}

void DrawLasers(void)
{
    for (int i = 0; i < MAX_LASERS; i++)
    {
        if (lasers[i].active)
        {
            DrawLine3D(lasers[i].start, lasers[i].end, lasers[i].color);
        }
    }
}
