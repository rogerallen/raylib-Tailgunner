
#include "enemy.h"
#include "projectile.h"
#include <math.h>

Enemy enemies[MAX_ENEMIES];

void InitEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].radius = 0.5f;
        enemies[i].color = BLUE;
    }
}

void SpawnWave(void)
{
    int waveSize = 3;
    int enemyIndex[waveSize];
    int found = 0;

    // Find enough inactive enemies for a wave
    for (int i = 0; i < MAX_ENEMIES && found < waveSize; i++)
    {
        if (!enemies[i].active)
        {
            enemyIndex[found] = i;
            found++;
        }
    }

    if (found == waveSize)
    {
        Vector3 waveCenter = { GetRandomValue(-15, 15), GetRandomValue(-15, 15), -100.0f };
        float speed = 0.25f;
        Vector3 direction = (Vector3){ -waveCenter.x, -waveCenter.y, -waveCenter.z };
        float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        Vector3 waveSpeed = { (direction.x / length) * speed, (direction.y / length) * speed, (direction.z / length) * speed };

        // Left ship
        enemies[enemyIndex[0]].position = (Vector3){ waveCenter.x - 2, waveCenter.y, waveCenter.z };
        enemies[enemyIndex[0]].speed = waveSpeed;
        enemies[enemyIndex[0]].active = true;

        // Center ship (leader)
        enemies[enemyIndex[1]].position = waveCenter;
        enemies[enemyIndex[1]].speed = waveSpeed;
        enemies[enemyIndex[1]].active = true;

        // Right ship
        enemies[enemyIndex[2]].position = (Vector3){ waveCenter.x + 2, waveCenter.y, waveCenter.z };
        enemies[enemyIndex[2]].speed = waveSpeed;
        enemies[enemyIndex[2]].active = true;
    }
}

int UpdateEnemies(Sound explosionSound) // FIXME add explosionSound usage
{
    int hits = 0;
    int activeEnemies = 0;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            activeEnemies++;

            enemies[i].position.x += enemies[i].speed.x;
            enemies[i].position.y += enemies[i].speed.y;
            enemies[i].position.z += enemies[i].speed.z;

            for (int j = 0; j < MAX_PROJECTILES; j++)
            {
                if (projectiles[j].active)
                {
                    if (CheckCollisionSpheres(enemies[i].position, enemies[i].radius, projectiles[j].position, projectiles[j].radius))
                    {
                        enemies[i].active = false;
                        projectiles[j].active = false;
                        hits++;
                    }
                }
            }

            if (enemies[i].position.z > 1.0f)
            {
                enemies[i].active = false;
            }
        }
    }

    if (activeEnemies == 0)
    {
        SpawnWave();
    }

    return hits;
}

void DrawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            DrawCube(enemies[i].position, enemies[i].radius * 2, enemies[i].radius * 2, enemies[i].radius * 2, enemies[i].color);
        }
    }
}
