#include "laser.h"
#include "enemy.h"
#include "raymath.h"

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

    Vector3 startPos[2];
    startPos[0] = (Vector3){ -1.0f, -1.0f, 0.0f };
    startPos[1] = (Vector3){ 1.0f, -1.0f, 0.0f };

    Vector3 target = Vector3Add(camera.position, Vector3Scale(ray.direction, 100.0f));

    for (int i = 0; i < MAX_LASERS; i++)
    {
        lasers[i].active = true;
        lasers[i].lifeTime = 0.1f;
        lasers[i].start = startPos[i];

        Vector3 direction = Vector3Normalize(Vector3Subtract(target, startPos[i]));
        Ray laserRay = { startPos[i], direction };

        float closestHitDist = 100.0f;
        Vector3 endPos = Vector3Add(lasers[i].start, Vector3Scale(direction, 100.0f));

        for (int j = 0; j < MAX_ENEMIES; j++)
        {
            if (enemies[j].active)
            {
                RayCollision collision = GetRayCollisionSphere(laserRay, enemies[j].position, enemies[j].radius);
                if (collision.hit && collision.distance < closestHitDist)
                {
                    closestHitDist = collision.distance;
                    endPos = collision.point;
                    enemies[j].active = false;
                    hits++;
                    PlaySound(explosionSound);
                }
            }
        }
        lasers[i].end = endPos;
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