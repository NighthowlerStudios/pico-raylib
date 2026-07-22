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

// Required for sleep and in-flash textures.
#include "pico/stdlib.h" 

// Memory reporting.
#include "sfe_pico.h"

#include "raylib.h"
#include <stdint.h>
#include <stddef.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>

#include "example_assets.h"

#define screenWidth 320
#define screenHeight 240

typedef enum 
{
    SIMPLE_SHAPES,
    BUNNIES,
    WAVING_CUBES,
    CUBICMAP,
    FIRST_PERSON_MAZE
} DemoModes;

double time = 0.0;
float rotation = 0.0f;
DemoModes currentMode = CUBICMAP;

void simple_shapes_draw(void)
{
    ClearBackground(RAYWHITE);

    DrawText("some basic shapes available on raylib", 20, 20, 10, DARKGRAY);
    DrawFPS(0, 0);

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
    //printf("[EXAMPLE] Rotation: %f\n", rotation);
    DrawPoly((Vector2){ screenWidth/4.0f*3, 175 }, 6, 40, rotation, BROWN);
    DrawPolyLines((Vector2){ screenWidth/4.0f*3, 175 }, 6, 45, rotation, BROWN);
    DrawPolyLinesEx((Vector2){ screenWidth/4.0f*3, 175 }, 6, 42, rotation, 6, BEIGE);

    // NOTE: We draw all LINES based shapes together to optimize internal drawing,
    // this way, all LINES are rendered in a single draw pass
    DrawLine(18, 42, screenWidth - 18, 42, BLACK);
}

void simple_shapes_update(void)
{
    // Delta timed, not using GetTime().
    if (GetFrameTime() == 0.0f) return;
    // 60 degrees per second.
    rotation += 60.0f * GetFrameTime();
}

#define MAX_BUNNIES 800 // Don't have much stack space on embedded.

typedef struct Bunny {
    Vector2 position;
    Vector2 speed;
    Color color;
} Bunny;

Bunny* bunnies = NULL;
int bunniesCount = 0;           // Bunnies counter
bool paused = false;

Texture2D texBunny;  // Doesn't contain the data.  RLSW will.

// Check for corruption across entire bunny array
// void check_all_bunnies(const char* phase)
// {
//     if (bunniesCount == 0) return;  // Don't check if no bunnies exist
    
//     for (int i = 0; i < bunniesCount; i++) {
//         int is_corrupted = (bunnies[i].position.x != bunnies[i].position.x) ||
//                           (bunnies[i].position.y != bunnies[i].position.y) ||
//                           (bunnies[i].speed.x != bunnies[i].speed.x) ||
//                           (bunnies[i].speed.y != bunnies[i].speed.y) ||
//                           (fabsf(bunnies[i].position.x) > 60000.0f) ||
//                           (fabsf(bunnies[i].position.y) > 60000.0f) ||
//                           (fabsf(bunnies[i].speed.x) > 30000.0f) ||
//                           (fabsf(bunnies[i].speed.y) > 30000.0f);
//         if (is_corrupted) {
//             printf("[CRITICAL] Corruption at %s (bunniesCount=%d)! Bunny %d at %p: pos=(%.2f, %.2f), speed=(%.2f, %.2f)\n",
//                    phase, bunniesCount, i, (void*)&bunnies[i], bunnies[i].position.x, bunnies[i].position.y, 
//                    bunnies[i].speed.x, bunnies[i].speed.y);
//         }
//     }
// }

void bunnymark_draw(void)
{
    // static int debug_once = 0;
    // if (!debug_once) {
    //     printf("[DEBUG] Bunnies array: %p to %p (%zu bytes)\n",
    //            (void*)bunnies, (void*)((uintptr_t)bunnies + MAX_BUNNIES * sizeof(Bunny)), 
    //            MAX_BUNNIES * sizeof(Bunny));
    //     printf("[DEBUG] texBunny at %p (id=%u, %dx%d)\n", 
    //            (void*)&texBunny, texBunny.id, texBunny.width, texBunny.height);
    //     debug_once = 1;
    // }
    
    //check_all_bunnies("START_DRAW");
    
    ClearBackground(RAYWHITE);

    for (int i = 0; i < bunniesCount; i++)
    {
        // These calls are not batched, it's all RLSW and less than 8000 units which 8192 is the default unit size.
        DrawTexture(texBunny, (int)bunnies[i].position.x, (int)bunnies[i].position.y, bunnies[i].color);
    }

    //check_all_bunnies("END_DRAW");
    
    DrawRectangle(0, 0, screenWidth, 40, BLACK);
    DrawText(TextFormat("bunnies: %i", bunniesCount), 120, 10, 20, GREEN);

    DrawFPS(0, 0);
}

void bunnymark_update(void)
{
    //check_all_bunnies("START_UPDATE");
    
    if (IsKeyPressed(KEY_A))
    {
        // Create more bunnies
        for (int i = 0; i < 10; i++)
        {
            if (bunniesCount < MAX_BUNNIES)
            {
                bunnies[bunniesCount].position.x = GetRandomValue(50, 200);
                bunnies[bunniesCount].position.y = GetRandomValue(50, 200);
                bunnies[bunniesCount].speed.x = (float)GetRandomValue(-250, 250);
                bunnies[bunniesCount].speed.y = (float)GetRandomValue(-250, 250);
                bunnies[bunniesCount].color = (Color){ GetRandomValue(50, 240),
                                                    GetRandomValue(80, 240),
                                                    GetRandomValue(100, 240), 255 };
                bunniesCount++;
            }
        }
    }

    // Update bunnies
    for (int i = 0; i < bunniesCount; i++)
    {
        float dt = GetFrameTime();
        bunnies[i].position.x += bunnies[i].speed.x * dt;
        bunnies[i].position.y += bunnies[i].speed.y * dt;

        if (((bunnies[i].position.x + (float)texBunny.width/2) > GetScreenWidth()) ||
            ((bunnies[i].position.x + (float)texBunny.width/2) < 0)) 
        {
            bunnies[i].speed.x *= -1;
            // Force to re-enter boundary
            bunnies[i].position.x += bunnies[i].speed.x*GetFrameTime();
        }
        if (((bunnies[i].position.y + (float)texBunny.height/2) > GetScreenHeight()) ||
            ((bunnies[i].position.y + (float)texBunny.height/2 - 40) < 0)) 
        {
            bunnies[i].speed.y *= -1;
            // Force to re-enter boundary
            bunnies[i].position.y += bunnies[i].speed.y*GetFrameTime();
        }
    }
    
    //check_all_bunnies("END_UPDATE");
}

Camera3D camera = { 0 };
float scale = 0.0f;

void waving_cubes_draw(void)
{
    // Specify the amount of blocks in each direction
    const int numBlocks = 9; // Exactly 729 cubes (4,374 triangles, around the N64 "limit" amount of 5,000).  
    // Not like the 15x15x15 = 3375 cubes in the original example, which is too much for the Pico to handle, and further from real-world.

    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

        DrawGrid(10, 5.0f);

        for (int x = 0; x < numBlocks; x++)
        {
            for (int y = 0; y < numBlocks; y++)
            {
                for (int z = 0; z < numBlocks; z++)
                {
                    // Scale of the blocks depends on x/y/z positions
                    float blockScale = (x + y + z)/30.0f;

                    // Scatter makes the waving effect by adding blockScale over time
                    float scatter = sinf(blockScale*20.0f + (float)(time*4.0f));

                    // Calculate the cube position
                    Vector3 cubePos = {
                        (float)(x - (float)numBlocks/2)*(scale*5.0f) + scatter,
                        (float)(y - (float)numBlocks/2)*(scale*5.0f) + scatter,
                        (float)(z - (float)numBlocks/2)*(scale*5.0f) + scatter
                    };

                    // Pick a color with a hue depending on cube position for the rainbow color effect
                    // NOTE: This function is quite costly to be done per cube and frame,
                    // pre-catching the results into a separate array could improve performance
                    Color cubeColor = ColorFromHSV((float)(((x + y + z)*18)%360), 0.75f, 0.9f);

                    // Calculate cube size
                    // Most pico screens are tiny!
                    float cubeSize = (2.4f - scale)*blockScale * 3;

                    // And finally, draw the cube!
                    DrawCube(cubePos, cubeSize, cubeSize, cubeSize, cubeColor);
                }
            }
        }

    EndMode3D();

    DrawFPS(10, 10);
}

// Old manual rotation method.
// void rotating_camera(void)
// {
//     // Move camera around the scene
//     double cameraTime = time*0.3;
//     camera.position.x = (float)cos(cameraTime)*40.0f;
//     camera.position.z = (float)sin(cameraTime)*40.0f;
// }

void waving_cubes_update(void)
{
    // Calculate time scale for cube position and size
    scale = (2.0f + (float)sin(time))*0.7f;

    UpdateCamera(&camera, CAMERA_ORBITAL);
}

Model model; // cubicmap model
Texture2D cubicmap;
Texture2D cubicmapAtlas;

Vector3 mapPosition = { -16.0f, 0.0f, -8.0f };          // Set model position

void cubicmap_update(void)
{
    UpdateCamera(&camera, CAMERA_ORBITAL);
}

#define MAZE_SCALE 2.0f

void cubicmap_draw(void)
{
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
        DrawModel(model, mapPosition, MAZE_SCALE, WHITE);  // draw with scale so it can be seen.
    EndMode3D();

    DrawTextureEx(cubicmap, (Vector2){ screenWidth - cubicmap.width*4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);
    DrawRectangleLines(screenWidth - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);

    DrawText("cubicmap image used to", 100, 190, 10, GRAY);
    DrawText("generate map 3d model", 100, 204, 10, GRAY);

    DrawFPS(10, 10);
}

Camera3D firstPersonCamera = { 0 };
int playerCellX = 0;
int playerCellY = 0;

void first_person_maze_update(void)
{
    Vector3 oldCamPos = firstPersonCamera.position;    // Store old camera position

    UpdateCamera(&firstPersonCamera, CAMERA_FIRST_PERSON);

    // Check player collision (we simplify to 2D collision detection)
    Vector2 playerPos = { firstPersonCamera.position.x, firstPersonCamera.position.z };
    float playerRadius = 0.1f;  // Collision radius (player is modelled as a cilinder for collision)

    playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
    playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);

    // Out-of-limits security check
    if (playerCellX < 0) playerCellX = 0;
    else if (playerCellX >= cubicmap.width) playerCellX = cubicmap.width - 1;

    if (playerCellY < 0) playerCellY = 0;
    else if (playerCellY >= cubicmap.height) playerCellY = cubicmap.height - 1;

    // Check map collisions using image data and player position against surrounding cells only
    for (int y = playerCellY - 1; y <= playerCellY + 1; y++)
    {
        // Avoid map accessing out of bounds
        if ((y >= 0) && (y < cubicmap.height))
        {
            for (int x = playerCellX - 1; x <= playerCellX + 1; x++)
            {
                // NOTE: Collision: Only checking R channel for white pixel
                if (((x >= 0) && (x < cubicmap.width)) &&
                    (bitmapCubicmap[y*cubicmap.width + x] == 255) &&
                    (CheckCollisionCircleRec(playerPos, playerRadius,
                    (Rectangle){ mapPosition.x - 0.5f + x*1.0f, mapPosition.z - 0.5f + y*1.0f, 1.0f, 1.0f })))
                {
                    // Collision detected, reset camera position
                    firstPersonCamera.position = oldCamPos;
                }
            }
        }
    }
}

void first_person_maze_draw(void)
{
    ClearBackground(RAYWHITE);

    BeginMode3D(firstPersonCamera);
        DrawModel(model, mapPosition, 1.0f, WHITE);                     // Draw maze map, unscaled
    EndMode3D();

    DrawTextureEx(cubicmap, (Vector2){ GetScreenWidth() - cubicmap.width*4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);
    DrawRectangleLines(GetScreenWidth() - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);

    // Draw player position radar
    DrawRectangle(GetScreenWidth() - cubicmap.width*4 - 20 + playerCellX*4, 20 + playerCellY*4, 4, 4, RED);

    DrawFPS(10, 10);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    // TODO: Support portrait windowing.

    // Also used to test that the screen gets clamped by force.
    // Note: to allocate this to SRAM, make sure InitWindow is first before all other allocations and that your stack is rather small!
    // For larger screens, it'll go to PSRAM instead.
    InitWindow(screenWidth, screenHeight, "pico raylib example");

    printf("[EXAMPLE] Total allocated after RLSW init: %d bytes\n", sfe_mem_used());

    // Set to 40 as a workaround to reduce LCD transmission stuttering.
    SetTargetFPS(40);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Note: RLSW processes colours on an array of four floats already.  You don't really lose performance since the buffer is truncated on the final pixel set, not the span interpolations.
    // NOTE: When doing &array to an array in flash, the copy is NOT done in Image.  So UnloadImage is unsafe.
    // NOTE: RLSW does not support compressed texture nor image data unless it was loaded by raylib as a file.
    Image imgBunny = {
        .data = &bitmapRaybunny,
        .width = 32,
        .height = 32,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
        .mipmaps = 1
    };
    // Copy from Flash to PSRAM.
    texBunny = LoadTextureFromImage(imgBunny);

    Image imgCubicmap = {
        .data = &bitmapCubicmap,
        .width = 32,
        .height = 16,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };
    // Copy from Flash to PSRAM.
    cubicmap = LoadTextureFromImage(imgCubicmap);

    printf("[EXAMPLE] Total allocated after bunny and cubicmap load: %d bytes\n", sfe_mem_used());

    Image imgCubicmapAtlas = {
        .data = &bitmapCubicmapAtlas,
        .width = 256,
        .height = 256,
        .format = PIXELFORMAT_UNCOMPRESSED_R5G6B5,
        .mipmaps = 1
    };
    // Copy from Flash to PSRAM.
    cubicmapAtlas = LoadTextureFromImage(imgCubicmapAtlas);

    printf("[EXAMPLE] Total allocated after maze texture load: %d bytes\n", sfe_mem_used());

    // Generate the maze mesh.
    // This is very memory intensive, almost 300KB of PSRAM.  Maybe further investigation, but I have a feeling it's just that much data for each triangle...
    Mesh mesh = GenMeshCubicmap(imgCubicmap, (Vector3){ 1.0f, 1.0f, 1.0f });
    model = LoadModelFromMesh(mesh);
    // Don't need to duplicate map pixels for collisions because they're still in flash forever.
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cubicmapAtlas;    // Set map diffuse texture

    printf("[EXAMPLE] Total allocated after maze mesh generate: %d bytes\n", sfe_mem_used());
    
    bunnies = RL_MALLOC(sizeof(Bunny) * MAX_BUNNIES);

    camera.position = (Vector3){ 30.0f, 20.0f, 30.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 70.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    firstPersonCamera.position = (Vector3){ 0.2f, 0.4f, 0.2f };    // Camera position
    firstPersonCamera.target = (Vector3){ 0.185f, 0.4f, 0.0f };    // Camera looking at point
    firstPersonCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    firstPersonCamera.fovy = 45.0f;                                // Camera field-of-view Y
    firstPersonCamera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    bool switchLock = false;

    printf("[EXAMPLE] Total allocated after all assets init: %d bytes\n", sfe_mem_used());

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        time = GetTime();

        if (IsKeyPressed(KEY_X) && !switchLock)
        {
            printf("[EXAMPLE] Switching modes.\n");
            currentMode++;
            if (currentMode > FIRST_PERSON_MAZE)
            {
                currentMode = SIMPLE_SHAPES;
            }

            switchLock = true;
        }
        else if (!IsKeyPressed(KEY_X))
        {
            switchLock = false;
        }

        // Update
        switch (currentMode)
        {
            case SIMPLE_SHAPES:
                simple_shapes_update();
                break;
            case BUNNIES:
                bunnymark_update();
                break;
            case WAVING_CUBES:
                waving_cubes_update();
                break;
            case CUBICMAP:
                cubicmap_update();
                break;
            case FIRST_PERSON_MAZE:
                first_person_maze_update();
                break;
            default:
                break;
        }

        // Check for corruption between update and draw
        // if (currentMode == BUNNIES) {
        //     check_all_bunnies("BETWEEN_UPDATE_DRAW");
        // }

        // Draw

        BeginDrawing();
            switch (currentMode)
            {
                case SIMPLE_SHAPES:
                    simple_shapes_draw();
                    break;
                case BUNNIES:
                    bunnymark_draw();
                    break;
                case WAVING_CUBES:
                    waving_cubes_draw();
                    break;
                case CUBICMAP:
                    cubicmap_draw();
                    break;
                case FIRST_PERSON_MAZE:
                    first_person_maze_draw();
                    break;
                default:
                    break;
            }            
        EndDrawing();

        printf("[EXAMPLE] Frame time: %f seconds\n", GetFrameTime());
    }

    UnloadModel(model);             // Unload map model
    UnloadTexture(cubicmap);        // Unload cubicmap texture
    UnloadTexture(cubicmapAtlas);   // Unload cubicmap atlas texture
    UnloadTexture(texBunny);        // Unload bunny texture

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}