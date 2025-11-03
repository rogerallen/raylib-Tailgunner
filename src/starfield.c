/*******************************************************************************************
*
*   starfield.c - Background starfield effect implementation
*
*   See starfield.h for module interface documentation.
*   
*   Implementation notes:
*   - Uses small 3D spheres for star representation
*   - Stars move at constant speed independent of frame rate
*   - Stars reset to front when passing behind camera
*   - Randomized alpha values create depth effect
*
*******************************************************************************************/

#include "starfield.h"

//----------------------------------------------------------------------------------
// Module Variables
//----------------------------------------------------------------------------------

Star stars[MAX_STARS];    // Array of star positions and colors

//----------------------------------------------------------------------------------
// Public Function Implementations (see starfield.h for documentation)
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// InitStarfield - Implementation Notes:
// - Randomly distributes stars in view volume
// - Sets varied alpha values for depth perception
// - Initial Z positions from -200 to 0
//----------------------------------------------------------------------------------
void InitStarfield(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].position = (Vector3){ GetRandomValue(-100, 100), GetRandomValue(-100, 100), GetRandomValue(-200, 0) };
        stars[i].color = (Color){ 255, 255, 255, GetRandomValue(100, 255) };
    }
}

//----------------------------------------------------------------------------------
// UpdateStarfield - Implementation Notes:
// - Moves stars at constant 60 units/second
// - Uses frame time for smooth movement
// - Resets stars to Z=0 when they pass Z=-200
//----------------------------------------------------------------------------------
void UpdateStarfield(void)
{
    // Make movement frame-rate independent
    const float speed = 60.0f; // units per second
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].position.z -= speed * dt; // Move towards negative Z (away from player)

        if (stars[i].position.z < -200.0f) // Reset when it passes behind the player
        {
            stars[i].position = (Vector3){ GetRandomValue(-100, 100), GetRandomValue(-100, 100), 0.0f }; // Reset in front of player
        }
    }
}

//----------------------------------------------------------------------------------
// DrawStarfield - Implementation Notes:
// - Renders each star as a small 3D sphere
// - Uses star's color property including alpha
// - Fixed size of 0.1 units for consistent appearance
//----------------------------------------------------------------------------------
void DrawStarfield(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        DrawSphere(stars[i].position, 0.1f, stars[i].color); // Draw as small spheres
    }
}