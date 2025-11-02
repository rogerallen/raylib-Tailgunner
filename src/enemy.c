#include "enemy.h"
#include "raymath.h"

Enemy enemies[MAX_ENEMIES];

void DrawEnemyShip(Enemy enemy);

// Function to calculate a point on a cubic Bezier curve
Vector3 GetCubicBezierPoint(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
{
    Vector3 result;
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    result.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    result.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
    result.z = uuu * p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

    return result;
}

void InitEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].radius = 1.0f;
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
        for (int i = 0; i < waveSize; i++)
        {
            enemies[enemyIndex[i]].active = true;
            enemies[enemyIndex[i]].t = 0.0f;

            float zOffset = i * -10.0f;
            int side = (i % 2 == 0) ? 1 : -1;

            enemies[enemyIndex[i]].p0 = (Vector3){ (float)GetRandomValue(-20, 20), (float)GetRandomValue(-20, 20), -100.0f + zOffset };
            enemies[enemyIndex[i]].p1 = (Vector3){ (float)GetRandomValue(-5, 5), (float)GetRandomValue(-5, 5), -50.0f + zOffset };
            enemies[enemyIndex[i]].p2 = (Vector3){ (float)GetRandomValue(-40, -20) * side, (float)GetRandomValue(10, 20), -25.0f };
            enemies[enemyIndex[i]].p3 = (Vector3){ (float)GetRandomValue(20, 40) * side, (float)GetRandomValue(-20, -10), 10.0f };
        }
    }
}

void UpdateEnemies(int* lives)
{
    int activeEnemies = 0;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            activeEnemies++;

            enemies[i].t += 0.0025f;
            enemies[i].position = GetCubicBezierPoint(enemies[i].p0, enemies[i].p1, enemies[i].p2, enemies[i].p3, enemies[i].t);

            if (enemies[i].t >= 1.0f)
            {
                enemies[i].active = false;
                (*lives)--;
            }
        }
    }

    if (activeEnemies == 0)
    {
        SpawnWave();
    }
}

void DrawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            DrawEnemyShip(enemies[i]);
        }
    }
}

void DrawEnemyShip(Enemy enemy)
{
    float r = enemy.radius;
    Vector3 pos = enemy.position;

    Vector3 v_top = { pos.x, pos.y + r, pos.z };
    Vector3 v_bottom = { pos.x, pos.y - r, pos.z };
    Vector3 v_right = { pos.x + r, pos.y, pos.z };
    Vector3 v_left = { pos.x - r, pos.y, pos.z };
    Vector3 v_front = { pos.x, pos.y, pos.z + r };
    Vector3 v_back = { pos.x, pos.y, pos.z - r };

    // Main body
    DrawLine3D(v_front, v_top, enemy.color);
    DrawLine3D(v_front, v_bottom, enemy.color);
    DrawLine3D(v_front, v_left, enemy.color);
    DrawLine3D(v_front, v_right, enemy.color);

    DrawLine3D(v_back, v_top, enemy.color);
    DrawLine3D(v_back, v_bottom, enemy.color);
    DrawLine3D(v_back, v_left, enemy.color);
    DrawLine3D(v_back, v_right, enemy.color);

    DrawLine3D(v_top, v_right, enemy.color);
    DrawLine3D(v_right, v_bottom, enemy.color);
    DrawLine3D(v_bottom, v_left, enemy.color);
    DrawLine3D(v_left, v_top, enemy.color);

    // Fins
    Vector3 fin_top_back = { pos.x, pos.y + r * 1.5f, pos.z - r * 1.5f };
    Vector3 fin_bottom_back = { pos.x, pos.y - r * 1.5f, pos.z - r * 1.5f };
    Vector3 fin_left_back = { pos.x - r * 1.5f, pos.y, pos.z - r * 1.5f };
    Vector3 fin_right_back = { pos.x + r * 1.5f, pos.y, pos.z - r * 1.5f };

    DrawLine3D(v_top, fin_top_back, enemy.color);
    DrawLine3D(v_back, fin_top_back, enemy.color);

    DrawLine3D(v_bottom, fin_bottom_back, enemy.color);
    DrawLine3D(v_back, fin_bottom_back, enemy.color);

    DrawLine3D(v_left, fin_left_back, enemy.color);
    DrawLine3D(v_back, fin_left_back, enemy.color);

    DrawLine3D(v_right, fin_right_back, enemy.color);
    DrawLine3D(v_back, fin_right_back, enemy.color);
}