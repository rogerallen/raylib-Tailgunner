#include "projectile.h"
#include <math.h>

Projectile projectiles[MAX_PROJECTILES];

void InitProjectiles(void)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        projectiles[i].active = false;
        projectiles[i].radius = 0.1f;
        projectiles[i].color = WHITE;
    }
}

void ShootProjectile(Ray ray)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!projectiles[i].active)
        {
            projectiles[i].position = (Vector3){ 0.0f, 0.0f, 0.0f };
            projectiles[i].active = true;

            float speed = 1.0f;
            projectiles[i].speed.x = ray.direction.x * speed;
            projectiles[i].speed.y = ray.direction.y * speed;
            projectiles[i].speed.z = ray.direction.z * speed;
            break;
        }
    }
}

void UpdateProjectiles(void)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            projectiles[i].position.x += projectiles[i].speed.x;
            projectiles[i].position.y += projectiles[i].speed.y;
            projectiles[i].position.z += projectiles[i].speed.z;

            if (projectiles[i].position.z < -100.0f)
            {
                projectiles[i].active = false;
            }
        }
    }
}

void DrawProjectiles(void)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            DrawSphere(projectiles[i].position, projectiles[i].radius, projectiles[i].color);
        }
    }
}