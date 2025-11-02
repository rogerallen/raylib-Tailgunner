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

    float closestHitDist = 100.0f;
    int closestEnemyIndex = -1;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            RayCollision collision = GetRayCollisionSphere(ray, enemies[i].position, enemies[i].radius);
            if (collision.hit && collision.distance < closestHitDist)
            {
                closestHitDist = collision.distance;
                closestEnemyIndex = i;
            }
        }
    }

    Vector3 endPos = Vector3Add(camera.position, Vector3Scale(ray.direction, 100.0f));

    if (closestEnemyIndex != -1)
    {
        endPos = enemies[closestEnemyIndex].position;
        enemies[closestEnemyIndex].active = false;
        hits++;
        PlaySound(explosionSound);
    }

    for (int i = 0; i < MAX_LASERS; i++)
    {
        lasers[i].active = true;
        lasers[i].lifeTime = 0.1f;
        lasers[i].start = startPos[i];
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
