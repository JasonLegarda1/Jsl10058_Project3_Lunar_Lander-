/**

* Author: Jason Legarda
* Assignment: Lunar Lander!
* Date due: 10/30/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.

**/


#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

enum GameStatus { PLAYING, MISSION_FAILED, MISSION_ACCOMPLISHED };

constexpr int NUMBER_OF_WORLD_OBJECTS = 2,
              WIN_PLATFORM_INDEX      = 0,
              FORBIDDEN_AREA_INDEX    = 1,
              FUEL_ICON_COUNT         = 10;

constexpr float ACCELERATION_OF_GRAVITY = 110.0f,
                HORIZONTAL_THRUST       = 100.0f,
                DRIFT_ACCELERATION      = 45.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                MAX_FUEL               = 100.0f,
                FUEL_USE_RATE           = 20.0f,
                FUEL_PER_ICON           = MAX_FUEL / FUEL_ICON_COUNT,
                PLAYER_SIZE             = 60.0f,
                PLAYER_TEXTURE_HEIGHT   = 120.0f,
                PLATFORM_MIN_X          = 350.0f,
                PLATFORM_MAX_X          = 750.0f,
                PLATFORM_SPEED          = 80.0f;

// Global Variables
AppStatus  gAppStatus  = RUNNING;
GameStatus gGameStatus = PLAYING;

float gPreviousTicks  = 0.0f,
      gTimeAccumulator = 0.0f,
      gFuel             = MAX_FUEL;

bool gUsingFuel = false;

Entity *gPlayer       = nullptr;
Entity *gWorldObjects = nullptr;

Texture2D gFieldTexture;
Texture2D gMissionFailedTexture;
Texture2D gMissionAccomplishedTexture;
Texture2D gFuelTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void renderFuel();
void renderMessage(Texture2D texture);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 3: Soccer Lander");

    gFieldTexture               = LoadTexture("assets/game/field.png");
    gMissionFailedTexture       = LoadTexture("assets/game/mission_failed.png");
    gMissionAccomplishedTexture = LoadTexture("assets/game/mission_accomplished.png");
    gFuelTexture                = LoadTexture("assets/game/fuel.png");

    std::map<Direction, std::vector<int>> ballFlameAnimation = {
        { RIGHT, { 0, 1, 2, 3 } }
    };

    gPlayer = new Entity(
        { ORIGIN.x, 80.0f },
        { PLAYER_SIZE, PLAYER_TEXTURE_HEIGHT },
        "assets/game/ball_flame_sprite_sheet.png",
        ATLAS,
        { 1.0f, 4.0f },
        ballFlameAnimation
    );
    gPlayer->setFrameSpeed(8);
    gPlayer->setColliderDimensions({ PLAYER_SIZE, PLAYER_SIZE });
    gPlayer->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    gPlayer->setEntityType(PLAYER);

    gWorldObjects = new Entity[NUMBER_OF_WORLD_OBJECTS];

    gWorldObjects[WIN_PLATFORM_INDEX].setTexture("assets/game/messi.png");
    gWorldObjects[WIN_PLATFORM_INDEX].setScale({ 150.0f, 210.0f });
    gWorldObjects[WIN_PLATFORM_INDEX].setColliderDimensions({ 100.0f, 170.0f });
    gWorldObjects[WIN_PLATFORM_INDEX].setPosition({ ORIGIN.x, 470.0f });
    gWorldObjects[WIN_PLATFORM_INDEX].setEntityType(WIN_PLATFORM);
    gWorldObjects[WIN_PLATFORM_INDEX].setSpeed(PLATFORM_SPEED);
    gWorldObjects[WIN_PLATFORM_INDEX].moveRight();

    gWorldObjects[FORBIDDEN_AREA_INDEX].setTexture("assets/game/opponent.png");
    gWorldObjects[FORBIDDEN_AREA_INDEX].setScale({ 130.0f, 180.0f });
    gWorldObjects[FORBIDDEN_AREA_INDEX].setColliderDimensions({ 100.0f, 165.0f });
    gWorldObjects[FORBIDDEN_AREA_INDEX].setPosition({ 150.0f, 475.0f });
    gWorldObjects[FORBIDDEN_AREA_INDEX].setEntityType(FORBIDDEN_AREA);

    gPreviousTicks = static_cast<float>(GetTime());
    SetTargetFPS(FPS);
}

void processInput()
{
    if (gGameStatus == PLAYING)
    {
        Vector2 acceleration = { 0.0f, ACCELERATION_OF_GRAVITY };
        Vector2 velocity     = gPlayer->getVelocity();
        gUsingFuel           = false;

        if (IsKeyDown(KEY_A) && gFuel > 0.0f)
        {
            acceleration.x = -HORIZONTAL_THRUST;
            gUsingFuel     = true;
        }
        else if (IsKeyDown(KEY_D) && gFuel > 0.0f)
        {
            acceleration.x = HORIZONTAL_THRUST;
            gUsingFuel     = true;
        }
        else if (velocity.x > 0.0f)
        {
            acceleration.x = -DRIFT_ACCELERATION;
        }
        else if (velocity.x < 0.0f)
        {
            acceleration.x = DRIFT_ACCELERATION;
        }

        gPlayer->setAcceleration(acceleration);
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (gGameStatus != PLAYING) return;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        if (gUsingFuel)
        {
            gFuel -= FUEL_USE_RATE * FIXED_TIMESTEP;

            if (gFuel <= 0.0f)
            {
                gFuel = 0.0f;
                gUsingFuel = false;

                Vector2 acceleration = gPlayer->getAcceleration();
                acceleration.x = 0.0f;
                gPlayer->setAcceleration(acceleration);
            }
        }

        gPlayer->update(
            FIXED_TIMESTEP,
            gWorldObjects,
            NUMBER_OF_WORLD_OBJECTS
        );

        if (gPlayer->missionAccomplished())
        {
            gGameStatus = MISSION_ACCOMPLISHED;
            break;
        }

        if (gPlayer->missionFailed())
        {
            gGameStatus = MISSION_FAILED;
            break;
        }

        Vector2 playerPosition = gPlayer->getPosition();
        float playerRadius = PLAYER_SIZE / 2.0f;

        if (playerPosition.x + playerRadius < 0.0f ||
            playerPosition.x - playerRadius > SCREEN_WIDTH ||
            playerPosition.y - playerRadius > SCREEN_HEIGHT)
        {
            gGameStatus = MISSION_FAILED;
            break;
        }

        Vector2 platformPosition =
            gWorldObjects[WIN_PLATFORM_INDEX].getPosition();

        if (platformPosition.x <= PLATFORM_MIN_X)
            gWorldObjects[WIN_PLATFORM_INDEX].moveRight();
        else if (platformPosition.x >= PLATFORM_MAX_X)
            gWorldObjects[WIN_PLATFORM_INDEX].moveLeft();

        for (int i = 0; i < NUMBER_OF_WORLD_OBJECTS; i++)
            gWorldObjects[i].update(FIXED_TIMESTEP, nullptr, 0);

        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void renderFuel()
{
    Rectangle fuelTextureArea = {
        0.0f,
        0.0f,
        static_cast<float>(gFuelTexture.width),
        static_cast<float>(gFuelTexture.height)
    };

    Rectangle fuelDestinationArea = { 20.0f, 20.0f, 90.0f, 30.0f };
    DrawTexturePro(
        gFuelTexture,
        fuelTextureArea,
        fuelDestinationArea,
        { 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    Texture2D ballTexture = gPlayer->getTexture();
    Vector2 ballSpriteSheetDimensions = gPlayer->getSpriteSheetDimensions();
    Rectangle ballTextureArea = getUVRectangle(
        &ballTexture,
        0,
        ballSpriteSheetDimensions.x,
        ballSpriteSheetDimensions.y
    );

    for (int i = 0; i < FUEL_ICON_COUNT; i++)
    {
        if (gFuel > i * FUEL_PER_ICON)
        {
            Rectangle fuelIconArea = {
                120.0f + i * 28.0f,
                8.0f,
                24.0f,
                48.0f
            };

            DrawTexturePro(
                ballTexture,
                ballTextureArea,
                fuelIconArea,
                { 0.0f, 0.0f },
                0.0f,
                WHITE
            );
        }
    }
}

void renderMessage(Texture2D texture)
{
    Rectangle textureArea = {
        0.0f,
        0.0f,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    Rectangle destinationArea = {
        ORIGIN.x,
        ORIGIN.y,
        700.0f,
        100.0f
    };

    DrawTexturePro(
        texture,
        textureArea,
        destinationArea,
        { 350.0f, 50.0f },
        0.0f,
        WHITE
    );
}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    Rectangle fieldTextureArea = {
        0.0f,
        0.0f,
        static_cast<float>(gFieldTexture.width),
        static_cast<float>(gFieldTexture.height)
    };

    Rectangle fieldDestinationArea = {
        0.0f,
        0.0f,
        static_cast<float>(SCREEN_WIDTH),
        static_cast<float>(SCREEN_HEIGHT)
    };

    DrawTexturePro(
        gFieldTexture,
        fieldTextureArea,
        fieldDestinationArea,
        { 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    for (int i = 0; i < NUMBER_OF_WORLD_OBJECTS; i++)
        gWorldObjects[i].render();

    gPlayer->render();
    renderFuel();

    if (gGameStatus == MISSION_FAILED)
        renderMessage(gMissionFailedTexture);
    else if (gGameStatus == MISSION_ACCOMPLISHED)
        renderMessage(gMissionAccomplishedTexture);

    EndDrawing();
}

void shutdown()
{
    delete gPlayer;
    delete[] gWorldObjects;

    UnloadTexture(gFieldTexture);
    UnloadTexture(gMissionFailedTexture);
    UnloadTexture(gMissionAccomplishedTexture);
    UnloadTexture(gFuelTexture);

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
