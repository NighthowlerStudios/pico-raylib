/*******************************************************************************************
*
*   raylib [shapes] example - basic shapes
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 1.0, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2014-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 320;
    const int screenHeight = 240;

    // Also used to test that the screen gets clamped by force.
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes");

    float rotation = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation += 0.2f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("some basic shapes available on raylib", 20, 20, 10, DARKGRAY);
            DrawFPS(20, 40);

            // Circle shapes and lines
            DrawCircle(screenWidth/5, 60, 17, DARKBLUE);
            DrawCircleGradient((Vector2){ screenWidth/5.0f, 110.0f }, 30, GREEN, SKYBLUE);
            DrawCircleLines(screenWidth/5, 170, 40, DARKBLUE);
            DrawEllipse(screenWidth/5, 60, 12, 10, YELLOW);
            DrawEllipseLines(screenWidth/5, 60, 15, 12, YELLOW);

            // Rectangle shapes and lines
            DrawRectangle(screenWidth/4*2 - 60, 50, 60, 30, RED);
            DrawRectangleGradientH(screenWidth/4*2 - 90, 85, 90, 65, MAROON, GOLD);
            DrawRectangleLines(screenWidth/4*2 - 40, 160, 40, 30, ORANGE);  // NOTE: Uses QUADS internally, not lines

            // Triangle shapes and lines
            DrawTriangle((Vector2){ screenWidth/4.0f *3.0f, 40.0f },
                         (Vector2){ screenWidth/4.0f *3.0f - 30.0f, 75.0f },
                         (Vector2){ screenWidth/4.0f *3.0f + 30.0f, 75.0f }, VIOLET);

            DrawTriangleLines((Vector2){ screenWidth/4.0f*3.0f, 80.0f },
                              (Vector2){ screenWidth/4.0f*3.0f - 10.0f, 115.0f },
                              (Vector2){ screenWidth/4.0f*3.0f + 10.0f, 115.0f }, DARKBLUE);

            // Polygon shapes and lines
            DrawPoly((Vector2){ screenWidth/4.0f*3, 175 }, 6, 40, rotation, BROWN);
            DrawPolyLines((Vector2){ screenWidth/4.0f*3, 175 }, 6, 45, rotation, BROWN);
            DrawPolyLinesEx((Vector2){ screenWidth/4.0f*3, 175 }, 6, 42, rotation, 6, BEIGE);

            // NOTE: We draw all LINES based shapes together to optimize internal drawing,
            // this way, all LINES are rendered in a single draw pass
            DrawLine(18, 42, screenWidth - 18, 42, BLACK);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}