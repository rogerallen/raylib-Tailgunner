#include "starfield.h"

Star stars[MAX_STARS];

void InitStarfield(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].position = (Vector3){ GetRandomValue(-100, 100), GetRandomValue(-100, 100), GetRandomValue(-200, 0) };
        stars[i].color = (Color){ 255, 255, 255, GetRandomValue(100, 255) };
    }
}

void UpdateStarfield(void)
{
    float speed = 1.0f; // Increased speed
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].position.z -= speed; // Move towards negative Z (away from player)

        if (stars[i].position.z < -200.0f) // Reset when it passes behind the player
        {
            stars[i].position = (Vector3){ GetRandomValue(-100, 100), GetRandomValue(-100, 100), 0.0f }; // Reset in front of player
        }
    }
}

void DrawStarfield(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        DrawSphere(stars[i].position, 0.1f, stars[i].color); // Draw as small spheres
    }
}