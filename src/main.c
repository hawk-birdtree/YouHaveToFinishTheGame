/*
 I want the pla*yer to have variable jump height
 I want the enemies to change direction when they collide with a wall, but I would have to refactor the enemies to be entities
 */

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

//***********************************CONTROLLERS********************************************

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// NOTE: Gamepad name ID depends on drivers and OS
#define XBOX360_LEGACY_NAME_ID  "Xbox Controller"
#if defined(PLATFORM_RPI)
#define XBOX360_NAME_ID     "Microsoft X-Box 360 pad"
#define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#else
#define XBOX360_NAME_ID     "Xbox 360 Controller"
#define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#endif

// Object storing inputs for Entity
typedef struct {
    float right;
    float left;
    float up;
    float down;
    bool jump;
} Input;

Input input;

//**************************************GLOBALS***********************************************

// Tile collision types
#define EMPTY   -1
#define BLOCK    0     // Start from zero, slopes can be added

// #define TILE_MAP_WIDTH        100
// #define TILE_MAP_HEIGHT        75
#define TILE_SIZE              16
#define TILE_ROUND             15

#define MAX_TILES              21
#define MAX_ROOMS               4
#define MAX_COLORS             12
#define MAX_PLAYER_HP           3
#define MAX_LEVEL_TEXTURES      6
#define TOTAL_LEVELS            MAX_LEVEL_TEXTURES

#define NUM_CIRCLES_IN_PILLAR_GROUP 4
#define NUM_PILLAR_GROUPS           3

#define NORMAL_SPEED   1.50*60 //1.5625f * 60
#define RUNNING_SPEED  2.50*60 //3.1255f * 60


#define SCREEN_WIDTH   1280 // 640 * 2
#define SCREEN_HEIGHT   768 // 480 * 2

int TILE_MAP_WIDTH = 0;
int TILE_MAP_HEIGHT = 0;

unsigned int current_level = 0;

size_t MAX_PROJECTILES         = 0;
size_t MAX_HORIZONTAL_MONSTERS = 0;
size_t MAX_VERTICAL_MONSTERS   = 0;
size_t MAX_TREASURE            = 0;
size_t MAX_CHECKPOINTS         = 0;
size_t MAX_SPIKES              = 0;

// char* current_level_texture[MAX_LEVEL_TEXTURES] = {"../out/level_1.png", "../out/level_2.png", "../out/level_3.png", "../out/level_4.png", "../out/level_5.png", "../out/level_6.png"};
char* current_level_texture[MAX_LEVEL_TEXTURES] = {"../out/levelBlockout.png"};

typedef enum {
    STATE_NORMAL,
    STATE_INVINCIBLE,
    STATE_ATTACKING,
    STATE_DEAD,
    // Add more states?
} PlayerState;

float delta     = 0.0f;
Sound soundCoin =  {0};
Sound soundFall =  {0};
Sound soundJump =  {0};

Texture2D levelSpriteSheet = {0};
Texture2D levelBlockout    = {0};
// Texture2D levelBlockoutBackground = {0};
// int tiles [TILE_MAP_WIDTH * TILE_MAP_HEIGHT] = {0};
int *tiles = NULL;

Image mapImage   = {0};
int tileType     = {0};
Color pixelColor = {0};
/*Color mapColors[TILE_MAP_WIDTH][TILE_MAP_HEIGHT] = {0}; */// Declare an array to store pre-processed colors
Color **mapColors = NULL;

int timer          = {0};
int timerDuration  = {0};
unsigned int score = {0};
bool timerActive   = false;
bool gameOver      = false;
bool win           = false;

unsigned int frameDelay        = {0};
unsigned int frameDelayCounter = {0};
unsigned int frameIndex        = {0};

typedef struct {
    int r, g, b;
    int count;
} ColorCount;

ColorCount colorCounts[MAX_COLORS] = {0};
int colorCountIndex = {0};

bool ColorMatches(ColorCount color, int r, int g, int b);
void IncrementColorCount(int r, int g, int b);
void CountColors(void);
void ResetVariables(void);

//*************************************CHECK COLLISION WITH WALL*********************************************

bool CheckWallAtPosition(int x, int y);

//**********************************************WORLD MAP****************************************************

typedef struct WorldMap{
    Vector2 position;
    Vector2 playerStartPos;
    Texture2D texture;
    Rectangle frame;
}WorldMap;

WorldMap worldmap = {0};
Rectangle worldMapSourceRect = {0};

int MapGetTileWorld(int x, int y);

void GetStageMapColors(void);
void DrawMap(void);
void InitWorldMap(void);
void UpdateWorldMap(void);
void DrawWorldMap(void);
int TileHeight(int y, int tile);
void UnloadMap(void);

//**********************************************TREASURE****************************************************
size_t treasureCount = 0;

typedef struct {
    Vector2 position;
    Rectangle frame;
    bool visible;
} Treasure;

Treasure *treasure = NULL;

void InitTreasure(void);
int  FindAvailableTreasureIndex(void);
void UpdateTreasure(void);
void DrawTreasure(void);
void UnloadTreasure(void);

//**********************************************SPIKES****************************************************
size_t spikeCount = 0;

typedef struct {
    Vector2 position;
    Texture2D texture;
    Rectangle frame;
    Rectangle collider;
}Spike;

Spike *spikes = NULL;

void InitSpikes(void);
int  FindAvailableSpikeIndex(void);
void CollisionSpikes(void);
void DrawSpikes(void);
void UnloadSpikes(void);

//**********************************************ROTATING PILLARS****************************************************
size_t rotatingPillarCount = 0;

typedef struct Pillar {
    float orbit_radius;
    float radius;
    Vector2 position;
}Pillar;

typedef struct PillarGroup {
    Vector2 center;
    float orbit_rotation;
    float rotation_speed;
    Pillar *pillars;
} PillarGroup;

PillarGroup pillarGroups[NUM_PILLAR_GROUPS] = {0};

void InitRotatingPillarGroups(void);
void UpdateRotatingPillarGroups(void);
void CollisionWithRotatingPillarGroups(void);
void DrawRotatingPillarGroups(void);
void UnloadPillars(void);

//**********************************************PROJECTILES****************************************************

typedef struct Projectile {
    Vector2 position;
    Vector2 velocity;
    Vector2 startPos;
    Vector2 endPos;
    int     maxDistance;
    float   cooldown;
    float   delay;
    float   radius;
    bool    active;
} Projectile;

Projectile *projectiles = NULL;
float shootCooldDown = 0.0f;

void InitProjectiles(void);
void SpawnProjectiles(void);
// Check if a projectile will hit a wall
bool CheckProjectileWallCollision(Projectile *proj);
void UpdateProjectile(float deltaTime);
void DrawProjectile(void);
void CheckProjectileCollisions(void);
void UnloadProjectiles(void);

//**********************************************ENTITIES****************************************************

// Physics body moving around
typedef struct {
    int       width;
    int       height;
    Vector2   position;
    float     direction;
    int       hp;
    Texture2D texture;
    int       state;
    int       iFrameTimer;
    Rectangle collider;
    Vector2   initialPosition;
    Vector2   lastCheckpoint;
    float     maxSpd;
    float     acc;
    float     dcc;
    float     gravity;
    float     jumpImpulse;
    float     jumpRelease;
    float     jumpTime;
    Vector2   velocity;
    float     hsp;
    float     vsp;
    bool      isGrounded;
    bool      isJumping;
    bool      hitOnFloor;
    bool      hitOnCeiling;
    bool      hitOnWall;
    Input    *control;
} Entity;

// Movement Functions Declaration (local)
void EntityMoveUpdate(Entity *intance);
void GetDirection(Entity *instance);
void GroundCheck(Entity *instance);
void MoveCalc(Entity *instance);
void GravityCalc(Entity *instance);
void CollisionCheck(Entity *instance);
void CollisionHorizontalBlocks(Entity *instance);
void CollisionVerticalBlocks(Entity *instance);

// Utility Functions Declaration (local)
int   ttc_sign(float x);
float ttc_abs(float x);
float ttc_clamp(float value, float min, float max);

//**********************************************PLAYER****************************************************

typedef enum {
    FACING_LEFT,
    FACING_RIGHT
} PlayerDirection;

enum PlayerAnimationState {
    ANIM_IDLE,
    ANIM_WALKING,
    ANIM_JUMPING,
    // Add more states as needed
};

Entity player = {0};
Vector2 player_animation_frame = {0};
int playerCount = 0;
unsigned int playerDeathCount = 0;
enum PlayerAnimationState currentAnimation = ANIM_IDLE;
PlayerDirection     playerDirection = FACING_RIGHT;
PlayerDirection lastPlayerDirection = FACING_RIGHT;

void InitPlayer(void);
void UpdatePlayerInput(void);
void UpdatePlayer(void);
void DrawPlayer(void);

//**********************************************CHECKPOINTS****************************************************
size_t checkpointCount = 0;

typedef struct{
    Vector2   position;
    Texture2D texture;
    Rectangle frame;
    Rectangle collider;
}Checkpoint;

Checkpoint *checkpoints = NULL;

void InitCheckpoints(void);
void DrawCheckPoints(void);
void CollisionCheckpoints(void);
int FindAvailableCheckpointIndex();
void UnloadCheckpoints(void);

//**********************************************CAMERA****************************************************

Camera2D camera = {0};
void InitCamera(void);
void CameraUpdate(void);

//**********************************************AUDIO******************************************************

void InitSounds(void);

//**********************************************MONSTERS******************************************************
size_t pathIndexHorizontal = 0;
size_t pathIndexVertical = 0;

typedef struct Monster {
    Vector2   position;
    int       direction;
    Texture2D texture;
    Rectangle frame;
    Rectangle collider;
    float     speed;
    int       movementDistance;
} Monster;

enum MonsterAnimationState {
    ANIM_MONSTER_IDLE,
    ANIM_MONSTER_MOVING,
    // Add more states as needed
};

enum MonsterAnimationState currentMonsterAnimation = ANIM_MONSTER_MOVING;

Monster *monsterHorizontal = NULL;
Monster *monsterVertical   = NULL;
Vector2 *monsterHorizontalPatrolPath = NULL;
Vector2 *monsterVerticalPatrolPath   = NULL;

const int MONSTER_MOVE_DISTANCE[] = {TILE_SIZE * 2, TILE_SIZE * 3, TILE_SIZE * 4, TILE_SIZE * 5, TILE_SIZE * 6};

void InitHorizontalMonsters(void);
void InitVerticalMonsters(void);
void UpdateHorizontalMonsters(void);
void UpdateVerticalMonsters(void);
void CollisionHorizontalMonsters(void);
void CollisionVerticalMonsters(void);
void UnloadMonsters(void);
bool CheckHorizontalMonsterWallCollision(Monster* monster, int direction);
bool CheckVerticalMonsterWallCollision(Monster* monster, int direction);

//***************************************GAME**********************************************

void LoadResources(void);
void SetGameState(void);
void PreprocessMapColors(void);
void InitGameComponents(void);
void InitGame(void);
void GameOver(void);
void ResetGame(void);
void UpdateGame(void);
void LoadNextLevel(void);
void DrawGame(void);
void UnloadGame(void);

//****************************************MAIN************************************************

int main(void)
{
    InitGame();

    int FPS = 60;
    delta = (float)1.0/(float)FPS;
    #if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, FPS, 1);
    #else
    SetTargetFPS(FPS);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateGame();
        DrawGame();
    }

    #endif
    UnloadGame();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

//******************************************************************************************

void ResetVariables(void) {
    playerCount             = 0;
    playerDeathCount        = 0;

    colorCountIndex  		= 0;

    timer            		= 0;
    timerDuration    		= 0;
    timerActive 	 		= false;

    gameOver         		= false;
    win              		= false;
    score            		= 0;

    spikeCount              = 0;
    treasureCount           = 0;
    checkpointCount         = 0;
    rotatingPillarCount     = 0;
    pathIndexHorizontal     = 0;
    pathIndexVertical       = 0;

    MAX_PROJECTILES         = 0;
    MAX_TREASURE     		= 0;
    MAX_CHECKPOINTS         = 0;
    MAX_SPIKES              = 0;
    MAX_HORIZONTAL_MONSTERS = 0;
    MAX_VERTICAL_MONSTERS   = 0;
}

//******************************************COLOR COUNTING***********************************************

bool ColorMatches(ColorCount color, int r, int g, int b) {
    return (color.r == r && color.g == g && color.b == b);
}

void IncrementColorCount(int r, int g, int b) {
    for (int i = 0; i < colorCountIndex; i++) {
        if (ColorMatches(colorCounts[i], r, g, b)) {
            colorCounts[i].count++;
            return;
        }
    }

    // If color is not found, add it to the array
    if (colorCountIndex < MAX_COLORS) {
        colorCounts[colorCountIndex].r = r;
        colorCounts[colorCountIndex].g = g;
        colorCounts[colorCountIndex].b = b;
        colorCounts[colorCountIndex].count = 1;
        colorCountIndex++;
    }
}

void CountColors(void) {

    MAX_PROJECTILES         = 0;
    MAX_TREASURE     		= 0;
    // MAX_CHECKPOINTS         = 0;
    // MAX_SPIKES              = 0;
    // MAX_HORIZONTAL_MONSTERS = 0;
    // MAX_VERTICAL_MONSTERS   = 0;

    // Iterate through the map image
    for (int y = 0; y < mapImage.height; y++) {
        for (int x = 0; x < mapImage.width; x++) {

            pixelColor = mapColors[x][y];

            IncrementColorCount(pixelColor.r, pixelColor.g, pixelColor.b);

            // Update MAX values based on specific colors
            if (       255 == pixelColor.r && 100 == pixelColor.g &&   0 == pixelColor.b) {
                MAX_HORIZONTAL_MONSTERS++;
            } else if (  0 == pixelColor.r && 162 == pixelColor.g && 232 == pixelColor.b) {
                MAX_VERTICAL_MONSTERS++;
            } else if (150 == pixelColor.r && 150 == pixelColor.g &&   0 == pixelColor.b) {
                MAX_TREASURE++;
            } else if (255 == pixelColor.r &&   0 == pixelColor.g &&   0 == pixelColor.b) {
                MAX_SPIKES++;
            } else if (  0 == pixelColor.r && 255 == pixelColor.g &&   0 == pixelColor.b) {
                MAX_CHECKPOINTS++;
            }else if ( 255 == pixelColor.r &&   0 == pixelColor.g && 255 == pixelColor.b) {
                MAX_PROJECTILES++;
            }
        }
    }

    // Display the results
    printf("Color counts:\n");
    for (int i = 0; i < colorCountIndex; i++) {
        printf("Color (%d, %d, %d): %d times\n",
               colorCounts[i].r,
               colorCounts[i].g,
               colorCounts[i].b,
               colorCounts[i].count);
    }

    // Log the updated MAX values
    printf("\n\n");
    printf("MAX_TREASURE: %zu\n", MAX_TREASURE);
    printf("MAX_SPIKES: %zu\n", MAX_SPIKES);
    printf("MAX_CHECKPOINTS: %zu\n", MAX_CHECKPOINTS);
    printf("MAX_HORIZONTAL_MONSTERS: %zu\n", MAX_HORIZONTAL_MONSTERS);
    printf("MAX_VERTICAL_MONSTERS: %zu\n", MAX_VERTICAL_MONSTERS);
    printf("MAX_PROJECTILES: %zu\n", MAX_PROJECTILES);
    printf("NUM_PILLAR_GROUPS: %d\n", NUM_PILLAR_GROUPS);
    printf("NUM_CIRCLES_IN_PILLAR_GROUP: %d\n", NUM_CIRCLES_IN_PILLAR_GROUP);
    printf("\n");
}

//**********************************************WORLD MAP****************************************************

void InitWorldMap(void){
    worldmap.position  = (Vector2){0.0f, 0.0f};
    worldmap.playerStartPos = (Vector2){0.0f, 0.0f};
    worldmap.texture   = LoadTexture(current_level_texture[current_level]);
    worldmap.frame     = (Rectangle){worldmap.position.x, worldmap.position.y,
        (float)worldmap.texture.width, (float)worldmap.texture.height};
}

void UpdateWorldMap(void) {
    worldmap.playerStartPos = (Vector2){player.position.x/TILE_SIZE, player.position.y/TILE_SIZE};
}

void DrawWorldMap(void) {

    Rectangle destRect = (Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    Vector2 origin = {0.0f, 0.0f};
    float rotation = 0.0f;
    Color tint = WHITE;

    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) {

        DrawTexturePro(worldmap.texture, worldmap.frame, destRect, origin, rotation, tint);

        int playerGridX = (int)(worldmap.playerStartPos.x);
        int playerGridY = (int)(worldmap.playerStartPos.y);

        float playerPixelX = playerGridX * 6.40f;// * TILE_SIZE;
        float playerPixelY = playerGridY * 6.40f;// * TILE_SIZE;

        DrawRectangle(playerPixelX, playerPixelY, TILE_SIZE/2, TILE_SIZE/2, DARKBROWN);
    }
}

void GetStageMapColors(void) {
    tiles = (int *)calloc(TILE_MAP_WIDTH * TILE_MAP_HEIGHT, sizeof(int));

    if(tiles == NULL){
        TraceLog(LOG_ERROR, "Failed to allocate memory for tiles Array!");
        CloseWindow();
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((pixelColor.r ==  0 && pixelColor.g ==  0 && pixelColor.b ==  0) ||
                (pixelColor.r == 20 && pixelColor.g == 20 && pixelColor.b == 20) ||
                (pixelColor.r == 40 && pixelColor.g == 40 && pixelColor.b == 40) ||
                (pixelColor.r == 60 && pixelColor.g == 60 && pixelColor.b == 60) ||
                (pixelColor.r == 80 && pixelColor.g == 80 && pixelColor.b == 80)) {
                tiles[x + y * TILE_MAP_WIDTH] = BLOCK;
                } else {
                    tiles[x + y * TILE_MAP_WIDTH] = EMPTY;
                }
        }
    }
}

void DrawMap(void){

    for (int y = 0; y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if (pixelColor.r == 0 && pixelColor.g == 0 && pixelColor.b == 0) {
                worldMapSourceRect = (Rectangle){0, 0, TILE_SIZE, TILE_SIZE};
                DrawTextureRec(levelSpriteSheet, worldMapSourceRect, (Vector2){(float)x*TILE_SIZE, (float)y*TILE_SIZE}, WHITE);
            }else if (pixelColor.r == 20 && pixelColor.g == 20 && pixelColor.b == 20) {
                worldMapSourceRect = (Rectangle){TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
                DrawTextureRec(levelSpriteSheet, worldMapSourceRect, (Vector2){(float)x*TILE_SIZE, (float)y*TILE_SIZE}, RED);
            }else if (pixelColor.r == 40 && pixelColor.g == 40 && pixelColor.b == 40) {
                worldMapSourceRect = (Rectangle){TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE};
                DrawTextureRec(levelSpriteSheet, worldMapSourceRect, (Vector2){(float)x*TILE_SIZE, (float)y*TILE_SIZE}, GREEN);
            }else if (pixelColor.r == 60 && pixelColor.g == 60 && pixelColor.b == 60) {
                worldMapSourceRect = (Rectangle){TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
                DrawTextureRec(levelSpriteSheet, worldMapSourceRect, (Vector2){(float)x*TILE_SIZE, (float)y*TILE_SIZE}, BLUE);
            }else if (pixelColor.r == 80 && pixelColor.g == 80 && pixelColor.b == 80) {
                worldMapSourceRect = (Rectangle){0, TILE_SIZE, TILE_SIZE, TILE_SIZE};
                DrawTextureRec(levelSpriteSheet, worldMapSourceRect, (Vector2){(float)x*TILE_SIZE, (float)y*TILE_SIZE}, ORANGE);
            }

            if(IsKeyDown(KEY_M) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
            {
                DrawText(TextFormat(" %d ", x), x*TILE_SIZE, y*TILE_SIZE, 1, GREEN);
                DrawText(TextFormat(" %d ", y), x*TILE_SIZE, y*TILE_SIZE+TILE_SIZE/2, 1, SKYBLUE);
            }
        }
    }
    // DrawTexture(levelBlockoutBackground, 0, 0, WHITE);
}

int MapGetTileWorld(int x, int y) {
    if (x < 0 || y < 0) return EMPTY;

    // Returns tile ID using world position
    x /= TILE_SIZE;
    y /= TILE_SIZE;

    if (x >= TILE_MAP_WIDTH || y >= TILE_MAP_HEIGHT) return EMPTY;

    return tiles[x+y*TILE_MAP_WIDTH];
}

int TileHeight(int y, int tile) {
    // Returns one pixel above solid. Extendable for slopes.
    switch(tile)
    {
        case EMPTY: break;
        case BLOCK: y = (y & ~TILE_ROUND) -1; break;
    }

    return y;
}

void UnloadMap(void){
    for(int i = 0; i < TILE_MAP_WIDTH; i++) {
        free(mapColors[i]);
        mapColors[i] = NULL;
    }
    free(mapColors);
    mapColors = NULL;

    free(tiles);
    tiles = NULL;
}

//**********************************************TREASURE****************************************************

void InitTreasure(void) {
    treasureCount = 0;
    treasure = (Treasure*)calloc(MAX_TREASURE, sizeof(Treasure));

    if(treasure == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Treasure Array!");
        CloseWindow();
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((150 == pixelColor.r && 150 == pixelColor.g && 0 == pixelColor.b) && treasureCount < MAX_TREASURE) {
                treasure[treasureCount].position = (Vector2){x * TILE_SIZE, y * TILE_SIZE};
                treasure[treasureCount].frame = (Rectangle){
                    levelSpriteSheet.width - levelSpriteSheet.width / 3.0f, 0,
                    levelSpriteSheet.width / 3.0f, levelSpriteSheet.height / 3.0f};
                    treasure[treasureCount].visible = true;
                    treasureCount++;
            }
        }
    }
}

void UpdateTreasure(void) {
    for (size_t i = 0; i < MAX_TREASURE; i++) {
        if (treasure[i].visible) {
            Rectangle treasureRect = (Rectangle){ treasure[i].position.x+3, treasure[i].position.y+3, TILE_SIZE-6, TILE_SIZE-6 };

            if (CheckCollisionRecs(player.collider, treasureRect)) {
                treasure[i].visible = false;
                score += 1;
                timer += 100;
                PlaySound(soundCoin);
            }
        }
    }

    win = (score == MAX_TREASURE);
}

void DrawTreasure(void) {
    for (size_t i = 0; i < MAX_TREASURE; i++) {
        Rectangle treasureRect = (Rectangle){ TILE_SIZE*2, TILE_SIZE, TILE_SIZE, TILE_SIZE };
        if (treasure[i].visible) {
            DrawTextureRec(levelSpriteSheet, treasureRect, treasure[i].position, GOLD);
            // DrawText(TextFormat("%d", i), treasure[i].position.x, treasure[i].position.y - 16, 8, WHITE);
        }
    }
}

int FindAvailableTreasureIndex(void){
    for (size_t i = 0; i < MAX_TREASURE; i++) {
        if (!treasure[i].visible) {
            return i; // Found an available treasure slot
        }
    }
    return -1; // No available treasure slots
}

void UnloadTreasure(void) {
    free(treasure);
    treasure = NULL;

    score = 0;
    treasureCount = 0;
}

//**********************************************SPIKES****************************************************

void InitSpikes(void) {
    spikes = (Spike*)calloc(MAX_SPIKES, sizeof(Spike));

    if(spikes == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Spike arrays!");
        CloseWindow();
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((255 == pixelColor.r && 0 == pixelColor.g && 0 == pixelColor.b) && spikeCount < MAX_SPIKES) {
                spikes[spikeCount].position = (Vector2){x * TILE_SIZE, y * TILE_SIZE};
                spikes[spikeCount].texture = levelSpriteSheet;
                spikes[spikeCount].frame = (Rectangle){TILE_SIZE+1, TILE_SIZE, TILE_SIZE, TILE_SIZE};
                spikes[spikeCount].collider = (Rectangle){
                    spikes[spikeCount].position.x + 1.0f, spikes[spikeCount].position.y + 3.0f,
                    spikes[spikeCount].frame.width - TILE_SIZE / 6.0f, spikes[spikeCount].frame.height - TILE_SIZE / 5.0f};
                    spikeCount++;
            }
        }
    }
}

void DrawSpikes(void){
    for (size_t i = 0; i < MAX_SPIKES; i++) {
        if (spikes[i].position.x != 0 && spikes[i].position.y != 0) {
            DrawTextureRec(spikes[i].texture, spikes[i].frame, spikes[i].position, LIGHTGRAY);
            // DrawText(TextFormat("%d", i), spikes[i].position.x, spikes[i].position.y - 16, 8, WHITE);
            //DrawRectangleLines(spikes[i].collider.x, spikes[i].collider.y, spikes[i].collider.width, spikes[i].collider.height, GREEN);
        }
    }
}

void CollisionSpikes(void) {
    for (size_t i = 0; i < MAX_SPIKES; i++) {
        if (player.state != STATE_INVINCIBLE && CheckCollisionRecs(player.collider, spikes[i].collider)) {
            player.position = player.lastCheckpoint;
            playerDeathCount += 1;
            player.hp -= 1;

            // Set I-frames
            player.state = STATE_INVINCIBLE;
            player.iFrameTimer = 240; // Number of frames for I-frames
            break;
        }
    }

    // Decrement the invincibility timer if the player is in the invincible state
    if (player.state == STATE_INVINCIBLE) {
        player.iFrameTimer--;
        if (player.iFrameTimer <= 0) {
            player.state = STATE_NORMAL; // Reset to normal state when I-frames expire
        }
    }
}

int FindAvailableSpikeIndex(){
    for (size_t i = 0; i < MAX_SPIKES; i++) {
        if (spikes[i].position.x == 0 && spikes[i].position.y == 0)
        {
            return i; // Found an available spike slot
        }
    }
    return -1; // No available spike slots
}

void UnloadSpikes(void) {
    free(spikes);
    spikes = NULL;
}

//**********************************************ROTATING PILLARS****************************************************

void InitRotatingPillarGroups(void) {

    rotatingPillarCount = 0;

    for(int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for(int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if((0 == pixelColor.r && 128 == pixelColor.g && 128 == pixelColor.b) && rotatingPillarCount < NUM_PILLAR_GROUPS) {
                pillarGroups[rotatingPillarCount].center = (Vector2){x * TILE_SIZE + TILE_SIZE/2.0f, y * TILE_SIZE + TILE_SIZE/2.0f};
                rotatingPillarCount++;
            }
        }
    }

    for(size_t n = 0; n < rotatingPillarCount && n < NUM_PILLAR_GROUPS; n++) {
        pillarGroups[n].orbit_rotation = 0.0f;
        pillarGroups[n].rotation_speed = 1.0f;

        pillarGroups[n].pillars = (Pillar*)calloc(NUM_CIRCLES_IN_PILLAR_GROUP, sizeof(Pillar));
        if(pillarGroups[n].pillars == NULL){
            TraceLog(LOG_ERROR, "Failed to allocate memory for Pillar Groups Array!");
            CloseWindow();
        }

        for (size_t i = 0; i < NUM_CIRCLES_IN_PILLAR_GROUP; i++) {
            pillarGroups[n].pillars[i].orbit_radius = 10.0f + (i * 8.0f);
            pillarGroups[n].pillars[i].radius = 4.0f;
            // pillarGroups[n].pillars[i].position = pillarGroups[0].center;
            pillarGroups[n].pillars[i].position = pillarGroups[n].center;
        }
    }
}

void UpdateRotatingPillarGroups(void) {
    for (size_t g = 0; g < rotatingPillarCount; g++) {
        // Update orbit rotation for each group
        pillarGroups[g].orbit_rotation += pillarGroups[g].rotation_speed * GetFrameTime() * 180.0f / PI;

        // Keep the angle in range [0, 360]
        if (pillarGroups[g].orbit_rotation >= 360.0f) {
            pillarGroups[g].orbit_rotation -= 360.0f;
        }

        // Update positions for each pillar in the group
        for (size_t i = 0; i < NUM_CIRCLES_IN_PILLAR_GROUP; i++) {
            float angleRad = pillarGroups[g].orbit_rotation * (PI / 180.0f); // Convert to radians
            pillarGroups[g].pillars[i].position.x = pillarGroups[g].center.x + pillarGroups[g].pillars[i].orbit_radius * cos(angleRad);
            pillarGroups[g].pillars[i].position.y = pillarGroups[g].center.y + pillarGroups[g].pillars[i].orbit_radius * sin(angleRad);
        }
    }
}

void CollisionWithRotatingPillarGroups(void) {
    for (size_t g = 0; g < rotatingPillarCount; g++) {
        if (player.state != STATE_INVINCIBLE) { // Make sure the player isn't invincible
            for (size_t i = 0; i < NUM_CIRCLES_IN_PILLAR_GROUP; i++) {
                if (CheckCollisionCircleRec(pillarGroups[g].pillars[i].position, pillarGroups[g].pillars[i].radius, player.collider)) {
                    // Deduct health only once, just like the spike collision
                    player.hp -= 1;

                    // Handle player position and death count
                    player.position = player.lastCheckpoint;
                    playerDeathCount += 1;

                    // Set invincibility frames
                    player.state = STATE_INVINCIBLE;
                    player.iFrameTimer = 240; // Reset invincibility timer to 240 frames

                    // Break after deducting health to ensure only one HP reduction per group
                    break; // Stop checking further pillars in this group
                }
            }
        }
    }
}

void DrawRotatingPillarGroups(void) {
    for (size_t g = 0; g < rotatingPillarCount; g++) {
        // Draw each pillar in the group
        for (size_t i = 0; i < NUM_CIRCLES_IN_PILLAR_GROUP; i++) {
            DrawCircleV(pillarGroups[g].pillars[i].position, pillarGroups[g].pillars[i].radius, YELLOW);
            DrawCircle(pillarGroups[g].center.x, pillarGroups[g].center.y, 6.0f, MAROON);
            // DrawText(TextFormat("%d", i), pillarGroups[g].pillars[i].position.x, pillarGroups[g].pillars[i].position.y, 8, WHITE);
        }
        // DrawText(TextFormat("%d", g), pillarGroups[g].center.x, pillarGroups[g].center.y, 8, WHITE);
    }
}

void UnloadPillars(void){

    for(size_t i = 0; i < rotatingPillarCount; i++){
        if(pillarGroups[i].pillars != NULL){
            free(pillarGroups[i].pillars);
            pillarGroups[i].pillars = NULL;
        }
    }
    rotatingPillarCount = 0;
}

//**********************************************PROJECTILES****************************************************

void InitProjectiles(void) {
    projectiles = (Projectile*)calloc(MAX_PROJECTILES, sizeof(Projectile));
    size_t numberOfProjectiles = 0;

    Vector2 projectile_direction[4] = {{80.0f, 0.0f}, {0.0f, 80.0f}, {-80.0f, 0.0f}, {0.0f, -80.0f}};

    if(projectiles == NULL){
        TraceLog(LOG_ERROR, "Failed to allocate memory for Projectiles Array!");
        CloseWindow();
    }

    for(int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for(int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {
            pixelColor = mapColors[x][y];

            if((255 == pixelColor.r && 0 == pixelColor.g && 255 == pixelColor.b) && numberOfProjectiles < MAX_PROJECTILES) {
                // Set the starting position
                projectiles[numberOfProjectiles].position = (Vector2){x * TILE_SIZE + (TILE_SIZE / 2.0f), y * TILE_SIZE + (TILE_SIZE / 2.0f)};
                projectiles[numberOfProjectiles].startPos = projectiles[numberOfProjectiles].position;
                projectiles[numberOfProjectiles].endPos = (Vector2){projectiles[numberOfProjectiles].position.x + TILE_SIZE * 4, projectiles[numberOfProjectiles].position.y}; // Example: move 4 tiles to the right

                // projectiles[numberOfProjectiles].velocity = (Vector2){80.0f, -40.0f}; // Example velocity (horizontal and vertical movement)
                projectiles[numberOfProjectiles].velocity = projectile_direction[1]; // [GetRandomValue(0,3)]; // (Vector2){80.0f, -40.0f}; // Example velocity (horizontal and vertical movement)
                projectiles[numberOfProjectiles].maxDistance = TILE_SIZE * 4;  // Distance to travel before reset
                projectiles[numberOfProjectiles].radius = 5.0f;
                projectiles[numberOfProjectiles].active = true;
                projectiles[numberOfProjectiles].delay = GetRandomValue(1.0f, 3.0f);
                projectiles[numberOfProjectiles].cooldown = 0.0f; // Initialize cooldown

                numberOfProjectiles++;
            }
        }
    }

    // Initialize other attributes
    for(size_t i = 0; i < MAX_PROJECTILES; i++){
        projectiles[i].cooldown = 0.0f; // Reset cooldown to 0
    }
}

// Check if a projectile will hit a wall
bool CheckProjectileWallCollision(Projectile *proj) {
    // Calculate the next position
    int nextX = (int)(proj->position.x + (proj->velocity.x > 0 ? proj->radius : -proj->radius));
    int nextY = (int)(proj->position.y + (proj->velocity.y > 0 ? proj->radius : -proj->radius));

    return CheckWallAtPosition(nextX, nextY);
}

void UpdateProjectile(float deltaTime) {
    for(size_t i = 0; i < MAX_PROJECTILES; i++) {
        if(projectiles[i].active) {
            // If there's a cooldown after resetting, we skip movement during the cooldown period
            if (projectiles[i].cooldown > 0.0f) {
                projectiles[i].cooldown -= deltaTime;
                continue;
            }

            if(projectiles[i].delay > 0.0f) {
                projectiles[i].delay -= deltaTime;
            } else {
                // Check for wall collision before moving
                if (CheckProjectileWallCollision(&projectiles[i])) {
                    // Hit a wall, reset to start
                    projectiles[i].cooldown = 1.0f;
                    projectiles[i].position = projectiles[i].startPos;
                    projectiles[i].delay = 1.0f;
                } else {
                    // Move in the direction of velocity
                    projectiles[i].position.x += projectiles[i].velocity.x * deltaTime;
                    projectiles[i].position.y += projectiles[i].velocity.y * deltaTime;
                }
            }
        }
    }
}

void DrawProjectile(void) {
    for(size_t i = 0; i < MAX_PROJECTILES; i++) {
        if(projectiles[i].active) {
            DrawCircleLines(projectiles[i].startPos.x, projectiles[i].startPos.y, projectiles[i].radius, RED);
            DrawCircleV(projectiles[i].position, projectiles[i].radius, RED);
            DrawCircleV(projectiles[i].startPos, projectiles[i].radius, BLACK);
            //DrawText(TextFormat("%d", i), projectiles[i].position.x - 16, projectiles[i].position.y - 4, 8, WHITE);
        }
    }
}

void CheckProjectileCollisions(void) {
    if(player.state != STATE_INVINCIBLE) {
        for(size_t i = 0; i < MAX_PROJECTILES; i++) {
            if(projectiles[i].active && CheckCollisionCircleRec(projectiles[i].position, projectiles[i].radius, player.collider)) {
                player.hp -= 1;

                // Handle player position and death count
                player.position = player.lastCheckpoint;
                playerDeathCount += 1;

                // Set invincibility frames
                player.state = STATE_INVINCIBLE;
                player.iFrameTimer = 240; // Reset invincibility timer to 240 frames
                break;
            }
        }
    }
}

void UnloadProjectiles(void) {
    free(projectiles);
    projectiles = NULL;
}

//**********************************************PLAYER****************************************************

void InitPlayer(void) {

    playerCount = 0;

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if (0 == pixelColor.r && 0 == pixelColor.g && 255 == pixelColor.b) {
                // Player Start Position
                if (playerCount == 0) {
                    player.position = (Vector2){TILE_SIZE * x, TILE_SIZE * y};
                    playerCount++;
                    if (playerCount > 1)
                    {
                        playerCount = 1;
                    }
                } else {
                    TraceLog(LOG_ERROR, "Multiple player start positions detected!");
                }
            }
        }
    }

    if (playerCount == 0) {
        TraceLog(LOG_ERROR, "No player start position detected!");
    }

    player.texture         = LoadTexture("../out/player.png");
    player.initialPosition = player.position;
    player.lastCheckpoint  = player.position;
    player.direction       = 1.0;
    player.maxSpd          = NORMAL_SPEED;
    player.hp              = MAX_PLAYER_HP;
    player.iFrameTimer     = STATE_NORMAL;
    player.state           = 0;
    player.acc             = 0.118164 * 60 * 60;
    player.dcc             = 0.2113281 * 60 * 60;
    player.gravity         = 0.363281 * 60 * 60;
    player.jumpImpulse     = -6.5625 * 60;
    player.jumpRelease     = player.jumpImpulse * 0.2f;
    player.jumpTime        = 0.0f;
    player.velocity        = (Vector2){0.0, 0.0};
    player.hsp             = 0;
    player.vsp             = 0;
    player.width           = player.texture.width / 8;
    player.height          = player.texture.height / 6;
    player.collider        = (Rectangle){player.position.x, player.position.y - 1, (float)player.width, (float)player.height};
    player.isGrounded      = false;
    player.isJumping       = false;
    player.hitOnFloor      = false;
    player.hitOnCeiling    = false;
    player.hitOnWall       = false;
    player.control         = &input;
}

void UpdatePlayerInput(void) {
    input.right = (float)(IsKeyDown('D')    || IsKeyDown(KEY_RIGHT) || IsGamepadButtonDown(   0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT));
    input.left  = (float)(IsKeyDown('A')    || IsKeyDown(KEY_LEFT)  || IsGamepadButtonDown(   0, GAMEPAD_BUTTON_LEFT_FACE_LEFT));
    input.down  = (float)(IsKeyDown('S')    || IsKeyDown(KEY_DOWN)  || IsGamepadButtonDown(   0, GAMEPAD_BUTTON_LEFT_FACE_UP));
    input.up    = (float)(IsKeyPressed('W') || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));

    //For jumping button needs to be toggled - allows pre-jump buffered (if held, jumps as soon as lands)
    if (input.up) input.jump = true;
    else if (input.up && GetFrameTime() >= 0.5f) input.jump = false;

    // Update current animation state based on game logic and collisions
    if (input.right) {
        currentAnimation = ANIM_WALKING;
        playerDirection = FACING_RIGHT;
        lastPlayerDirection = FACING_RIGHT;
    } else if (input.left) {
        currentAnimation = ANIM_WALKING;
        playerDirection = FACING_LEFT;
        lastPlayerDirection = FACING_LEFT;
    } else if (input.up && player.velocity.y > 0.0f) {
        currentAnimation = ANIM_JUMPING;
        player.velocity = (Vector2){ player.velocity.x, player.velocity.y * 0.5f };
    } else {
        currentAnimation = ANIM_IDLE; // Player's x position is not changing
    }

    if(player.hp <= 0) {
        player.hp = 0;
    }

    player.collider.x = player.position.x - TILE_SIZE / 4.0f;
    player.collider.y = player.position.y - TILE_SIZE / 2.0f - 1;
}

void UpdatePlayer(void) {
    UpdatePlayerInput();
    EntityMoveUpdate(&player);
}

void DrawPlayer(void) {
    frameDelayCounter++; // Increment the frame delay counter

    // Check if it's time to update the frame
    if (frameDelayCounter >= frameDelay) {
        player_animation_frame.x += player.texture.width / 4.0f;

        // Check if the animation looped
        if (player_animation_frame.x >= player.texture.width) {
            player_animation_frame.x = 0;
        }

        frameDelayCounter = 0; // Reset the frame delay counter
    }

    // Calculate the source rectangle based on the current frame
    Rectangle playerFrameRect = {player_animation_frame.x, 0, player.texture.width / 4.0f, player.texture.height / 4.0f};
    Vector2 position = (Vector2){player.position.x - player.texture.width / 8.0f, player.position.y - player.texture.height / 4.0f + 1};
    Color tint = WHITE;

    if(player.state == STATE_INVINCIBLE) {
        tint = GRAY;
    }

    if (player.isGrounded) {
        // The player is on the ground, so you can reset to IDLE or WALKING as needed.
        if (player.velocity.x != 0) {
            currentAnimation = ANIM_WALKING;
        } else {
            currentAnimation = ANIM_IDLE;
        }
    } else {
        // The player is in the air, maintain the JUMPING animation state.
        currentAnimation = ANIM_JUMPING;
    }

    // Check player direction based on input
    if (playerDirection == FACING_LEFT) {
        // Flip the texture when moving left
        playerFrameRect.width = -playerFrameRect.width;
    }

    switch (currentAnimation) {
        case ANIM_IDLE:
            // Render the idle animation frames
            DrawTextureRec(player.texture, playerFrameRect, position, tint);
            break;

        case ANIM_WALKING:
            // Render the walking animation frames
            playerFrameRect.y = player.texture.height / 4.0f - player.texture.height;
            DrawTextureRec(player.texture, playerFrameRect, position, tint);
            break;

        case ANIM_JUMPING:
            // Render the jumping animation frames
            playerFrameRect.y = player.texture.height / 4.0f * 3;
            DrawTextureRec(player.texture, playerFrameRect, position, tint);
            break;

            // Add cases for other animation states as needed
    }

    //DrawRectangleLines(player.collider.x, player.collider.y, player.collider.width, player.collider.height, BLUE);
}

//**********************************************ENTITIES****************************************************

// Main Entity movement calculation
void EntityMoveUpdate(Entity *instance) {
    GroundCheck(instance);
    GetDirection(instance);
    MoveCalc(instance);
    GravityCalc(instance);
    CollisionCheck(instance);

    // Horizontal velocity together including last frame sub-pixel value
    float xVel = instance->velocity.x * delta + instance->hsp;
    // Horizontal velocity in pixel values
    int xsp = (int)ttc_abs(xVel) * ttc_sign(xVel);
    // Save horizontal velocity sub-pixel value for next frame
    instance->hsp = instance->velocity.x * delta - xsp;

    // Vertical velocity together including last frame sub-pixel value
    float yVel = instance->velocity.y * delta + instance->vsp;
    // Vertical velocity in pixel values
    int ysp = (int)ttc_abs(yVel) * ttc_sign(yVel);
    // Save vertical velocity sub-pixel value for next frame
    instance->vsp = instance->velocity.y * delta - ysp;

    // Add pixel value velocity to the position
    instance->position.x += xsp;
    instance->position.y += ysp;

    // Prototyping Safety net - keep in view
    instance->position.x = ttc_clamp(instance->position.x, 0.0, TILE_MAP_WIDTH * (float)TILE_SIZE);
    instance->position.y = ttc_clamp(instance->position.y, 0.0, TILE_MAP_HEIGHT * (float)TILE_SIZE);
}

// Physics functions

// Read Input for horizontal movement direction
void GetDirection(Entity *instance) {
    instance->direction = (instance->control->right - instance->control->left);
}

// Check pixel bellow to determine if Entity is grounded
void GroundCheck(Entity *instance) {
    int x = (int)instance->position.x;
    int y = (int)instance->position.y + 1;
    instance->isGrounded = false;

    // Center point check
    int c = MapGetTileWorld(x , y);

    if (c != EMPTY) {
        int h = TileHeight(y, c);
        instance->isGrounded = (y >= h);
    }

    if (!instance->isGrounded) {
        // Left bottom corner check
        int xl = (x - instance->width / 2);
        int l = MapGetTileWorld(xl , y);

        if (l != EMPTY) {
            int h = TileHeight(y, l);
            instance->isGrounded = (y >= h);
        }

        if (!instance->isGrounded) {
            // Right bottom corner check
            int xr = (x + instance->width / 2 - 1);
            int r = MapGetTileWorld(xr , y);
            if (r != EMPTY) {
                int h = TileHeight(y, r);
                instance->isGrounded = (y >= h);
            }
        }
    }
}

// Simplified horizontal acceleration / deacceleration logic
void MoveCalc(Entity *instance) {
    // Check if direction value is above dead zone - direction is held
    float deadZone = 0.0;
    if (ttc_abs(instance->direction) > deadZone) {
        instance->velocity.x += instance->direction*instance->acc*delta;
        instance->velocity.x = ttc_clamp(instance->velocity.x, -instance->maxSpd, instance->maxSpd);
    } else {
        // No direction means deacceleration
        float xsp = instance->velocity.x;
        if (ttc_abs(0 - xsp) < instance->dcc*delta) instance->velocity.x = 0;
        else if (xsp > 0) instance->velocity.x -= instance->dcc*delta;
        else instance->velocity.x += instance->dcc*delta;
    }
}

void Jump(Entity *instance) {
    instance->velocity.y = instance->jumpImpulse;
    instance->isJumping = true;
    instance->isGrounded = false;
    instance->jumpTime = 0.0f; // Reset jumpTime
}

// Gravity calculation and Jump detection
void GravityCalc(Entity *instance) {
    static bool wasGrounded = false;

    if (instance->isGrounded) {
        if (instance->isJumping) {
            instance->isJumping = false;
            instance->control->jump = false;    // Cancel input button
        } else if (!instance->isJumping && instance->control->jump) {
            Jump(instance);
            PlaySound(soundJump);
        }

        if(!wasGrounded) {
            PlaySound(soundFall);
        }
        wasGrounded = true;

    } else {
        if (instance->isJumping) {
            if (!instance->control->jump) {
                instance->isJumping = false;

                if (instance->velocity.y < instance->jumpRelease) {
                    instance->velocity.y = instance->jumpRelease;
                }
            }
        }
        wasGrounded = false;
    }

    // Add gravity
    instance->velocity.y += instance->gravity*delta;

    // Limit falling to negative jump value
    if (instance->velocity.y > -instance->jumpImpulse) {
        instance->velocity.y = -instance->jumpImpulse;
    }

    // Inside your GravityCalc function
    if (IsKeyDown(KEY_LEFT_SHIFT) ||IsKeyDown(KEY_RIGHT_SHIFT)|| IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        // Increase the player's speed to make them run
        player.maxSpd = RUNNING_SPEED;
    } else {
        // Reset the maximum speed to normal value
        player.maxSpd = NORMAL_SPEED;
    }
}

// Main collision check function
void CollisionCheck(Entity *instance) {
    CollisionHorizontalBlocks(instance);
    CollisionVerticalBlocks(instance);
}

// Detect and solve horizontal collision with block tiles
void CollisionHorizontalBlocks(Entity *instance) {
    // Get horizontal speed in pixels
    float xVel = instance->velocity.x*delta + instance->hsp;
    int xsp = (int)ttc_abs(xVel)*ttc_sign(xVel);

    instance->hitOnWall = false;

    // Get bounding box side offset
    int side;
    if (xsp > 0) side = instance->width / 2 - 1;
    else if (xsp < 0) side = -instance->width / 2;
    else return;

    int x = (int)instance->position.x;
    int y = (int)instance->position.y;
    int mid = -instance->height / 2;
    int top = -instance->height + 1;

    // 3 point check
    int b = MapGetTileWorld(x + side + xsp , y) > EMPTY;
    int m = MapGetTileWorld(x + side + xsp , y + mid) > EMPTY;
    int t = MapGetTileWorld(x + side + xsp , y + top) > EMPTY;

    // If implementing slopes it's better to disable b and m, if (x,y) is in the slope tile
    if (b || m || t) {
        if (xsp > 0) x = ((x + side + xsp) & ~TILE_ROUND) - 1 - side;
        else x = ((x + side + xsp) & ~TILE_ROUND) + TILE_SIZE - side;

        instance->position.x = (float)x;
        instance->velocity.x = 0.0;
        instance->hsp = 0.0;
        instance->hitOnWall = true;
    }
}

// Detect and solve vertical collision with block tiles
void CollisionVerticalBlocks(Entity *instance) {
    // Get vertical speed in pixels
    float yVel = instance->velocity.y*delta + instance->vsp;
    int ysp = (int)ttc_abs(yVel)*ttc_sign(yVel);
    instance->hitOnCeiling = false;
    instance->hitOnFloor = false;

    // Get bounding box side offset
    int side = 0;
    if (ysp > 0) side = 0;
    else if (ysp < 0) side = -instance->height + 1;
    else return;

    int x = (int)instance->position.x;
    int y = (int)instance->position.y;
    int xl = -instance->width/2;
    int xr = instance->width/2 - 1;

    int c = MapGetTileWorld(x , y + side + ysp) > EMPTY;
    int l = MapGetTileWorld(x + xl , y + side + ysp) > EMPTY;
    int r = MapGetTileWorld(x + xr , y + side + ysp) > EMPTY;

    if (c || l || r) {
        if (ysp > 0) {
            y = ((y + side + ysp) & ~TILE_ROUND) - 1 - side;
            instance->hitOnFloor = true;
        } else {
            y = ((y + side + ysp) & ~TILE_ROUND) + TILE_SIZE - side;
            instance->hitOnCeiling = true;
        }

        instance->position.y = (float)y;
        instance->velocity.y = 0.0;
        instance->vsp = 0.0;
    }
}

// Return sign of the floal as int (-1, 0, 1)
int ttc_sign(float x) {
    if (x < 0) return -1;
    else if (x < 0.0001) return 0;
    else return 1;
}

// Return absolute value of float
float ttc_abs(float x) {
    if (x < 0.0) x *= -1.0;
    return x;
}

// Clamp value between min and max
float ttc_clamp(float value, float min, float max) {
    const float res = value < min ? min : value;
    return res > max ? max : res;
}

//**********************************************CHECKPOINTS****************************************************

void InitCheckpoints(void) {
    checkpoints = (Checkpoint*)calloc(MAX_CHECKPOINTS, sizeof(Checkpoint));

    if(checkpoints == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Checkpoints Array!");
        CloseWindow();
    }

    for(size_t i = 0; i < MAX_CHECKPOINTS; i++) {
        checkpoints[i].frame.y = TILE_SIZE * 2;
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((0 == pixelColor.r && 255 == pixelColor.g && 0 == pixelColor.b) && checkpointCount < MAX_CHECKPOINTS) {
                checkpoints[checkpointCount].position = (Vector2){x * TILE_SIZE, y * TILE_SIZE};
                checkpoints[checkpointCount].texture = levelSpriteSheet;
                checkpoints[checkpointCount].frame = (Rectangle){0, TILE_SIZE * 2, TILE_SIZE, TILE_SIZE};
                checkpoints[checkpointCount].collider = (Rectangle){
                    checkpoints[checkpointCount].position.x, checkpoints[checkpointCount].position.y,
                    checkpoints[checkpointCount].frame.width, checkpoints[checkpointCount].frame.height};
                    checkpointCount++;
            }
        }
    }
}

void DrawCheckPoints(void) {
    for (size_t i = 0; i < MAX_CHECKPOINTS; i++) {
        if (checkpoints[i].position.x != 0 && checkpoints[i].position.y != 0) {
            DrawTextureRec(checkpoints[i].texture, checkpoints[i].frame, checkpoints[i].position, WHITE);
            // DrawText(TextFormat("%d", i), checkpoints[i].position.x, checkpoints[i].position.y - 16, 8, WHITE);
            //DrawRectangleLines(checkpoints[i].position.x, checkpoints[i].position.y, checkpoints[i].frame.width, checkpoints[i].frame.height, GREEN);
            //DrawRectangleLines(checkpoints[i].collider.x, checkpoints[i].collider.y, checkpoints[i].frame.width, checkpoints[i].frame.height, ORANGE);
        }
    }
}

void CollisionCheckpoints(void) {
    for(size_t i = 0; i < MAX_CHECKPOINTS; i++) {
        if(CheckCollisionRecs(player.collider, checkpoints[i].collider)) {
            player.lastCheckpoint = checkpoints[i].position;
            checkpoints[i].frame.y = TILE_SIZE * 3;
        }
    }
}

int FindAvailableCheckpointIndex() {
    for (size_t i = 0; i < MAX_CHECKPOINTS; i++) {
        if (checkpoints[i].position.x == 0 && checkpoints[i].position.y == 0) {
            return i; // Found an available checkpoint slot
        }
    }
    return -1; // No available checkpoint slots
}

void UnloadCheckpoints(void) {
    free(checkpoints);
    checkpoints = NULL;
}

//*****************************************CAMERA************************************************

void InitCamera(void) {
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }; // center of screen
    camera.target = (Vector2){ 0.0f, 0.0f }; // will be updated per room
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;
}

void CameraUpdate(void) {
    // Get visible area size in world units
    float visibleWidth  = SCREEN_WIDTH  / camera.zoom;
    float visibleHeight = SCREEN_HEIGHT / camera.zoom;

    // Determine which screen-sized room the player is in
    int roomX = player.position.x / visibleWidth;
    int roomY = player.position.y / visibleHeight;

    // Set camera to center on that room
    camera.target.x = roomX * visibleWidth + visibleWidth / 2.0f;
    camera.target.y = roomY * visibleHeight + visibleHeight / 2.0f;
}

//**********************************************AUDIO******************************************************

void InitSounds(void) {
    soundCoin = LoadSound("../out/pickup_coin.wav");
    soundJump = LoadSound("../out/jump_1.wav");
    soundFall = LoadSound("../out/fall_2.wav");
    SetSoundVolume(soundCoin, 0.1f);
    SetSoundVolume(soundJump, 0.1f);
    SetSoundVolume(soundFall, 0.2f);
}

//**********************************************MONSTERS******************************************************

void InitHorizontalMonsters(void) {
    pathIndexHorizontal = 0;

    monsterHorizontal = (Monster*)calloc(MAX_HORIZONTAL_MONSTERS, sizeof(Monster));

    if(monsterHorizontal == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Monster horizontal array!");
        CloseWindow();
    }

    monsterHorizontalPatrolPath = (Vector2*)calloc(MAX_HORIZONTAL_MONSTERS, sizeof(Vector2));

    if(monsterHorizontalPatrolPath == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Monster Horizontal Path array!");
        CloseWindow();
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((255 == pixelColor.r && 100 == pixelColor.g && 0 == pixelColor.b) && pathIndexHorizontal < MAX_HORIZONTAL_MONSTERS) {
                monsterHorizontalPatrolPath[pathIndexHorizontal++] = (Vector2){x * TILE_SIZE, y * TILE_SIZE};
            }
        }
    }

    for (size_t i = 0; i < MAX_HORIZONTAL_MONSTERS && i < pathIndexHorizontal; i++) {
        monsterHorizontal[i].position  = monsterHorizontalPatrolPath[i];
        monsterHorizontal[i].direction = 0;
        monsterHorizontal[i].texture   = levelSpriteSheet;
        monsterHorizontal[i].frame     = (Rectangle){TILE_SIZE+1, TILE_SIZE, TILE_SIZE, TILE_SIZE};
        monsterHorizontal[i].collider  = (Rectangle){monsterHorizontal[i].position.x, monsterHorizontal[i].position.y, monsterHorizontal[i].frame.width, monsterHorizontal[i].frame.height / 2};
        monsterHorizontal[i].speed     = 1; // You can add random speed if desired
        monsterHorizontal[i].movementDistance = 7; // MONSTER_MOVE_DISTANCE[GetRandomValue(0,4)];
    }
}

void InitVerticalMonsters(void) {
    pathIndexVertical = 0;

    monsterVertical = (Monster*)calloc(MAX_VERTICAL_MONSTERS, sizeof(Monster));

    if(monsterVertical == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Monster Vertical array!");
        CloseWindow();
    }

    monsterVerticalPatrolPath = (Vector2*)calloc(MAX_VERTICAL_MONSTERS, sizeof(Vector2));

    if(monsterVerticalPatrolPath == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Monster Vertical Path array!");
        CloseWindow();
    }

    for (int y = 0; y < mapImage.height && y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < mapImage.width && x < TILE_MAP_WIDTH; x++) {

            pixelColor = mapColors[x][y];

            if ((0 == pixelColor.r && 162 == pixelColor.g && 232 == pixelColor.b) && pathIndexVertical < MAX_VERTICAL_MONSTERS) {
                monsterVerticalPatrolPath[pathIndexVertical++] = (Vector2){x * TILE_SIZE, y * TILE_SIZE};
            }
        }
    }

    for (size_t i = 0; i < MAX_VERTICAL_MONSTERS && i < pathIndexVertical; i++) {
        monsterVertical[i].position  = monsterVerticalPatrolPath[i];
        monsterVertical[i].direction = 0;
        monsterVertical[i].texture   = levelSpriteSheet;
        monsterVertical[i].frame     = (Rectangle){TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE};
        monsterVertical[i].collider  = (Rectangle){monsterVertical[i].position.x, monsterVertical[i].position.y, monsterVertical[i].frame.width, monsterVertical[i].frame.height / 2};
        monsterVertical[i].speed     = 1; // You can add random speed if desired
        monsterVertical[i].movementDistance = 7; // MONSTER_MOVE_DISTANCE[GetRandomValue(0,4)];
    }
}

void UpdateHorizontalMonsters(void) {
    CollisionHorizontalMonsters();

    // Update animation frames globally for horizontal monsters
    frameDelayCounter++;
    if (frameDelayCounter >= frameDelay) {
        for(size_t i = 0; i < pathIndexHorizontal; i++) {
            monsterHorizontal[i].frame.x = TILE_SIZE * 3;
            monsterHorizontal[i].frame.y += TILE_SIZE;
            if (monsterHorizontal[i].frame.y >= TILE_SIZE * 2) {
                monsterHorizontal[i].frame.y = 0;
            }
        }
        frameDelayCounter = 0;
    }

    for (size_t i = 0; i < pathIndexHorizontal; i++) {
        // Moving left
        if (monsterHorizontal[i].direction == 1) {
            // Check for wall before moving
            if (CheckHorizontalMonsterWallCollision(&monsterHorizontal[i], -1)) {
                // Hit a wall, change direction
                monsterHorizontal[i].direction = -1;
                monsterHorizontal[i].frame.width = -monsterHorizontal[i].frame.width;
            } else {
                monsterHorizontal[i].position.x -= monsterHorizontal[i].speed;
            }
        }
        // Moving right
        else {
            // Check for wall before moving
            if (CheckHorizontalMonsterWallCollision(&monsterHorizontal[i], 1)) {
                // Hit a wall, change direction
                monsterHorizontal[i].direction = 1;
                monsterHorizontal[i].frame.width = -monsterHorizontal[i].frame.width;
            } else {
                monsterHorizontal[i].position.x += monsterHorizontal[i].speed;
            }
        }

        monsterHorizontal[i].collider.x = monsterHorizontal[i].position.x;
        monsterHorizontal[i].collider.y = monsterHorizontal[i].position.y + TILE_SIZE / 3.0f;
    }
}

// REPLACE your UpdateVerticalMonsters function with this:
void UpdateVerticalMonsters(void) {
    CollisionVerticalMonsters();

    // Update animation frames globally for vertical monsters
    frameDelayCounter++;
    if (frameDelayCounter >= frameDelay) {
        for (size_t i = 0; i < pathIndexVertical; i++) {
            monsterVertical[i].frame.x = TILE_SIZE;
            monsterVertical[i].frame.y += TILE_SIZE;
            if (monsterVertical[i].frame.y > TILE_SIZE * 3) {
                monsterVertical[i].frame.y = TILE_SIZE * 2;
            }
        }
        frameDelayCounter = 0;
    }

    for (size_t i = 0; i < pathIndexVertical; i++) {
        // Moving up
        if (monsterVertical[i].direction == 1) {
            // Check for wall before moving
            if (CheckVerticalMonsterWallCollision(&monsterVertical[i], -1)) {
                // Hit a wall, change direction
                monsterVertical[i].direction = -1;
                monsterVertical[i].frame.height = -monsterVertical[i].frame.height;
            } else {
                monsterVertical[i].position.y -= monsterVertical[i].speed;
            }
        }
        // Moving down
        else {
            // Check for wall before moving
            if (CheckVerticalMonsterWallCollision(&monsterVertical[i], 1)) {
                // Hit a wall, change direction
                monsterVertical[i].direction = 1;
                monsterVertical[i].frame.height = -monsterVertical[i].frame.height;
            } else {
                monsterVertical[i].position.y += monsterVertical[i].speed;
            }
        }

        monsterVertical[i].collider.x = monsterVertical[i].position.x;
        monsterVertical[i].collider.y = monsterVertical[i].position.y + TILE_SIZE / 3.0f;
    }
}

void DrawHorizontalMonsters(void) {
    for (size_t i = 0; i < pathIndexHorizontal; i++) {
        // Use global monster_frame.x to set the current animation frame
        Rectangle frameRect = {
            monsterHorizontal[i].frame.x,               // Current frame X position (updated globally)
            monsterHorizontal[i].frame.y,                             // Y position for the texture
            monsterHorizontal[i].frame.width, // Frame width (positive or negative for flipping)
            monsterHorizontal[i].frame.height // Frame height
        };

        // Draw the monster using the calculated frame rectangle
        DrawTextureRec(monsterHorizontal[i].texture, frameRect, monsterHorizontal[i].position, WHITE);
        // DrawText(TextFormat("%d", i), monsterHorizontal[i].position.x, monsterHorizontal[i].position.y - 16, 8, WHITE);
    }
}

void DrawVerticalMonsters(void) {
    for (size_t i = 0; i < pathIndexVertical; i++) {
        // Use global monster_frame.x to set the current animation frame
        Rectangle frameRect = {
            monsterVertical[i].frame.x,               // Current frame X position (updated globally)
            monsterVertical[i].frame.y,                             // Y position for the texture
            monsterVertical[i].frame.width, // Frame width (positive or negative for flipping)
            monsterVertical[i].frame.height // Frame height
        };

        // Draw the monster using the calculated frame rectangle
        DrawTextureRec(monsterVertical[i].texture, frameRect, monsterVertical[i].position, GOLD);
        // DrawText(TextFormat("%d", i), monsterVertical[i].position.x, monsterVertical[i].position.y - 16, 8, WHITE);
    }
}

void CollisionHorizontalMonsters(void) {

    for (size_t i = 0; i < (MAX_HORIZONTAL_MONSTERS); i++) {
        if (player.state != STATE_INVINCIBLE && CheckCollisionRecs(player.collider, monsterHorizontal[i].collider)) {
            player.position = player.lastCheckpoint;
            playerDeathCount += 1;
            player.hp -= 1;
            player.state = STATE_INVINCIBLE;
            player.iFrameTimer = 240;
            break;
        }
    }

    if (player.state == STATE_INVINCIBLE) {
        player.iFrameTimer--;
        if (player.iFrameTimer <= 0) {
            player.state = STATE_NORMAL;
        }
    }
}

void CollisionVerticalMonsters(void) {

    for (size_t i = 0; i < (MAX_VERTICAL_MONSTERS); i++) {
        if (player.state != STATE_INVINCIBLE && CheckCollisionRecs(player.collider, monsterVertical[i].collider)) {
            player.position = player.lastCheckpoint;
            playerDeathCount += 1;
            player.hp -= 1;
            player.state = STATE_INVINCIBLE;
            player.iFrameTimer = 240;
            break;
        }
    }

    if (player.state == STATE_INVINCIBLE) {
        player.iFrameTimer--;
        if (player.iFrameTimer <= 0) {
            player.state = STATE_NORMAL;
        }
    }
}

bool CheckWallAtPosition(int x, int y) {
    int tile = MapGetTileWorld(x, y);
    return (tile == BLOCK);
}

// Check for wall collision for horizontal monsters
bool CheckHorizontalMonsterWallCollision(Monster *monster, int direction) {
    int x = (int)monster->position.x;
    int y = (int)monster->position.y;

    // Check multiple points (top, middle, bottom of sprite)
    int checkX = (direction > 0) ? x + TILE_SIZE : x - 1;
    int topY = y + 1;
    int midY = y + TILE_SIZE / 2;
    int botY = y + TILE_SIZE - 1;

    return CheckWallAtPosition(checkX, topY) ||
    CheckWallAtPosition(checkX, midY) ||
    CheckWallAtPosition(checkX, botY);
}

// Check for wall collision for vertical monsters
bool CheckVerticalMonsterWallCollision(Monster *monster, int direction) {
    int x = (int)monster->position.x;
    int y = (int)monster->position.y;

    // Check multiple points (left, middle, right of sprite)
    int checkY = (direction > 0) ? y + TILE_SIZE : y - 1;
    int leftX = x + 1;
    int midX = x + TILE_SIZE / 2;
    int rightX = x + TILE_SIZE - 1;

    return CheckWallAtPosition(leftX, checkY) ||
    CheckWallAtPosition(midX, checkY) ||
    CheckWallAtPosition(rightX, checkY);
}

void UnloadMonsters(void) {
    free(monsterHorizontal);
    monsterHorizontal = NULL;
    free(monsterVertical);
    monsterVertical = NULL;
    free(monsterHorizontalPatrolPath);
    monsterHorizontalPatrolPath = NULL;
    free(monsterVerticalPatrolPath);
    monsterVerticalPatrolPath = NULL;

    MAX_HORIZONTAL_MONSTERS = 0;
    MAX_VERTICAL_MONSTERS   = 0;
}

//****************************************GAME***********************************************************

void LoadResources(void) {
    levelSpriteSheet        = LoadTexture("../out/spritesheet.png");
    levelBlockout           = LoadTexture("../out/levelBlockout.png"); // LoadTexture(current_level_texture[current_level]);
    // levelBlockoutBackground = LoadTexture("../out/bg_level_blockout.png");
    mapImage                = LoadImage("../out/levelBlockout.png");// LoadImage(current_level_texture[current_level]);
    UnloadImage(mapImage);
}

void SetGameState(void) {
    gameOver          = false;
    win               = false;
    playerDeathCount  = 0;
    timer             = 10000;
    timerActive       = true;
    score             = 0;
    player_animation_frame     = (Vector2){0, 0};
    frameDelay        = 10;
    frameDelayCounter = 0;
    frameIndex        = 0;
}

void PreprocessMapColors(void) {

    mapImage = LoadImage(current_level_texture[current_level]);

    for (int y = 0; y < mapImage.height; y++) {
        for (int x = 0; x < mapImage.width; x++) {
            mapColors[x][y] = GetImageColor(mapImage, x, y); // Store color in the array
        }
    }
    UnloadImage(mapImage);
}

void InitGameComponents(void) {

    TILE_MAP_WIDTH = mapImage.width;
    TILE_MAP_HEIGHT = mapImage.height;
    mapColors = (Color**)calloc(TILE_MAP_WIDTH, sizeof(Color*));

    if(mapColors == NULL){
        TraceLog(LOG_ERROR, "Failed to allocate memory for mapColor width Array!");
        CloseWindow();
    }

    for(int i = 0; i < TILE_MAP_WIDTH; i++){
        mapColors[i] = (Color*)calloc(TILE_MAP_HEIGHT, sizeof(Color));
        if(mapColors[i] == NULL) {
            TraceLog(LOG_ERROR, "Failed to allocate memory for mapColor height Array!");
            fprintf(stderr, "memory allocation failed for columns(y) in rows(x) %d\n", i);
            for(int j = 0; j < i; j++){
                free(mapColors[j]);
            }
            free(mapColors);
            CloseWindow();
        }
    }

    for(int j = 0; j < TILE_MAP_HEIGHT; j++) {
        for(int i = 0; i < TILE_MAP_WIDTH; i++) {
            mapColors[i][j] = (Color){0, 0, 0, 255};
        }
    }

    printf("Map Width %d, Map Height %d", TILE_MAP_WIDTH, TILE_MAP_HEIGHT);

    PreprocessMapColors();
    CountColors();

    InitCamera();
    InitSounds();
    InitRotatingPillarGroups();
    InitProjectiles();
    InitSpikes();
    InitTreasure();
    InitCheckpoints();
    InitHorizontalMonsters();
    InitVerticalMonsters();
    InitWorldMap();
    InitPlayer();
    GetStageMapColors();
}

void InitGame(void) {

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "You have to finish the game");

    InitAudioDevice();
    HideCursor();
    SetGameState();
    LoadResources();
    InitGameComponents();
}

void ResetGame(void) {
    ResetVariables();

    UnloadTreasure();
    UnloadSpikes();
    UnloadCheckpoints();
    UnloadMonsters();
    UnloadPillars();
    UnloadProjectiles();
    UnloadMap();

    SetGameState();
    InitGameComponents();
}

void GameOver(void) {
    gameOver        = true;
    player.velocity = (Vector2){0.0f, 0.0f};
    player.position = (Vector2){TILE_SIZE * -5, TILE_SIZE * -5}; /*(Vector2){player.position.x/TILE_SIZE, player.position.y/TILE_SIZE};*/
    Color tint      = {0,0,0,180};
    DrawRectangle(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, tint);

    if(timer == 0) {
        DrawText("TIME'S UP!", SCREEN_WIDTH/4, SCREEN_HEIGHT/2.5, 80, RED);
    }
    else {
        DrawText("GAME 0VER!", SCREEN_WIDTH/4, SCREEN_HEIGHT/3, 80, RED);
        DrawText(TextFormat("Time: %d", timer/10), SCREEN_WIDTH/3, SCREEN_HEIGHT/1.5, 50, BLUE);
    }

    DrawText(TextFormat("Deaths: %d", playerDeathCount), GetScreenWidth()/2.3f - MeasureText("Deaths:  ", 20)/2.0f, GetScreenHeight()/1.1f - 50, 30, DARKGREEN);
    DrawText("Press ESC, or Triangle/Y button or R key", SCREEN_WIDTH/4.0f, SCREEN_HEIGHT/1.75f, 20, WHITE);
}

void UpdateGame(void) {

    UpdatePlayer();

    float deltaTime = GetFrameTime();
    UpdateProjectile(deltaTime);
    UpdateRotatingPillarGroups();
    UpdateHorizontalMonsters();
    UpdateVerticalMonsters();
    UpdateTreasure();
    UpdateWorldMap();

    CollisionCheckpoints();
    CollisionSpikes();
    CollisionWithRotatingPillarGroups();
    CheckProjectileCollisions();

    CameraUpdate();

    if(player.hp <= 0 || timer <= 0) {
        GameOver();
    }

    if (win) {
        if (IsKeyPressed(KEY_R) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
            if(current_level >= TOTAL_LEVELS){
                current_level = -1;
                // ResetGame();
            }
            else {
                win = false; //reset win before loading next level
                LoadNextLevel();
                // score = 0;
                // treasureCount = 0;
            }
        }
    }

    if (timerActive) {
        timer--;

        if(timer <= 0 || true == win || player.hp <= 0) {
            timerActive = false;
            timer = timer;
        }
    }
}

void LoadNextLevel(void){
    current_level++;

    if(current_level >= TOTAL_LEVELS){
        current_level = 0; //show end screen
    }

    UnloadTreasure();
    UnloadSpikes();
    UnloadCheckpoints();
    UnloadMonsters();
    UnloadPillars();
    UnloadProjectiles();

    UnloadTexture(levelSpriteSheet);
    UnloadTexture(levelBlockout);
    // UnloadTexture(levelBlockoutBackground);
    UnloadMap();

    LoadResources();
    SetGameState();
    InitGameComponents();
}

void DrawGame(void) {
    BeginDrawing();

    BeginMode2D(camera);

    ClearBackground(BLACK);

    DrawMap();
    DrawRotatingPillarGroups();
    DrawProjectile();
    DrawTreasure();
    DrawSpikes();
    DrawCheckPoints();
    DrawHorizontalMonsters();
    DrawVerticalMonsters();
    DrawPlayer();

    EndMode2D();

    if ((!timerActive && false == win) || player.hp <= 0) {
        if(gameOver) {
            GameOver();

            if(IsKeyPressed(KEY_R) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
                ResetGame();
            }
        }
    }

    DrawRectangle(SCREEN_WIDTH/2.2, 8, 160, 32, BLACK );
    DrawText(TextFormat("Time:%d", timer/10),SCREEN_WIDTH/2.2, 10, 30, GOLD);

    //Graphical Unit Interface GUI/HUD Heads Up Display
    Rectangle frame    = (Rectangle){TILE_SIZE*2, TILE_SIZE, TILE_SIZE, TILE_SIZE};
    Rectangle destRect = (Rectangle){8, 8, TILE_SIZE*2, TILE_SIZE*2.3};
    Vector2 origin     = (Vector2){0.0f, 0.0f};
    float rotation     = 0.0f;

    DrawRectangle(5, 5, TILE_SIZE*7, TILE_SIZE*2.5, BLACK);
    for(int i = 0; i < player.hp; i++) {
        if (i >= MAX_PLAYER_HP || player.hp < 0) {
            // Avoid accessing elements outside the bounds of your health array
            // and ensure health doesn't go below zero.
            //this does nothing. why not consider removing it?
            break;
        }
        DrawTexturePro(player.texture, (Rectangle){0.0f,0.0f,player.width*2.0f, player.height/1.25f},(Rectangle){SCREEN_WIDTH/5.0f + (i*48), SCREEN_HEIGHT/50.0f, (float)player.texture.width, player.texture.height/2.0f}, (Vector2){0.0f,0.0f}, 0.0f, WHITE);
    }

    DrawTexturePro(levelSpriteSheet, frame, destRect, origin, rotation, GOLD);
    DrawWorldMap();
    DrawText(TextFormat("%i/%d", score, MAX_TREASURE), TILE_SIZE*2.7, TILE_SIZE, 22, WHITE);

    if (win) {
        player.velocity = (Vector2){0.0f,0.0f};
        DrawText("WINNER!", GetScreenWidth()/2.7f - MeasureText("WINNER!", 20)/2.0f, GetScreenHeight()/3.0f - 50, 60, DARKBLUE);
        DrawText("PRESS [R] or (Y/Triangle) TO PLAY AGAIN", GetScreenWidth()/2.0f - MeasureText("PRESS [R] or (Y/Triangle) TO PLAY AGAIN", 20)/2.0f, GetScreenHeight()/2.0f - 50, 20, WHITE);

        if(playerDeathCount) {
            DrawText(TextFormat("Deaths: %d", playerDeathCount), GetScreenWidth()/2.2f - MeasureText("Deaths:  ", 20)/2.0f, GetScreenHeight()/1.7f - 50, 30, DARKGREEN);
        } else {
            DrawText(TextFormat("!PERFECT, RUN!", playerDeathCount), GetScreenWidth()/2.4f - MeasureText("!PERFECT, RUN!", 20)/2.0f, GetScreenHeight()/1.7f - 50, 30, GREEN);
        }
    }

    EndDrawing();
}

void UnloadGame(void) {
    UnloadSound(soundCoin);
    UnloadSound(soundJump);
    UnloadSound(soundFall);

    UnloadTreasure();
    UnloadSpikes();
    UnloadCheckpoints();
    UnloadMonsters();
    UnloadPillars();
    UnloadProjectiles();

    UnloadTexture(levelSpriteSheet);
    UnloadTexture(levelBlockout);
    // UnloadTexture(levelBlockoutBackground);
    UnloadTexture(player.texture);
    UnloadMap();
}
