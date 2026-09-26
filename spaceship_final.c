#include "raylib.h"
#include "raymath.h"
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

#define screenwidth  1500
#define screenlength  900

typedef enum
{
    MENU,
    NAME_ENTRY,
    PLAYING,
    HOW_TO_PLAY,
    GAME_OVER,
    HALL_OF_FAME
} GameState;

Music menu_music;
Music bg_music;
Music howToPlayMusic;
Sound bullet_shoot;
Sound astro_bul_col;
Sound astro_ship_col;
bool soundOn=true;

void InitAudio(void){
    InitAudioDevice();
    menu_music = LoadMusicStream("audio/tokyorifft-interstellar-374344.mp3");
    bg_music = LoadMusicStream("audio/atlasaudio-ambient-astronomy-511860.mp3");
    howToPlayMusic = LoadMusicStream("audio/Viktor Kraus - Blueberries.mp3");
    bullet_shoot = LoadSound("audio/freesound_community-fire-88783.mp3");
    astro_bul_col = LoadSound("audio/dragon-studio-explosion-sound-effect-425455.mp3");
    astro_ship_col = LoadSound("audio/finntastico-asteroid-hitting-something-152511.mp3");

    PlayMusicStream(menu_music);
}

void unload_audio(void){
    UnloadMusicStream(menu_music);
    UnloadMusicStream( bg_music);
    UnloadSound( bullet_shoot);
    UnloadSound(astro_bul_col);
    UnloadSound(astro_ship_col);

    CloseAudioDevice();
}

typedef struct{
    Vector2 position;
    Vector2 velocity;
    float life;
    float max_life;
    bool active;
} fire_particle;
#define max_fire_particle 200
fire_particle fire_particles[max_fire_particle];


void spawn_particles(Vector2 spaceship_position , Vector2 spaceship_direction ){
    for(int i = 0 ; i < max_fire_particle ; i++){
        if(!fire_particles[i].active){
            Vector2 fire_backward = Vector2Scale(spaceship_direction , -1);
            float fire_spread = ((float)GetRandomValue(-30 , 30)) * DEG2RAD;
            Vector2 fire_spread_direction = { fire_backward.x * cosf(fire_spread) - fire_backward.y * sinf(fire_spread) , fire_backward.x * sinf(fire_spread) + fire_backward.y * cosf(fire_spread) };
            float fire_speed = GetRandomValue(100,250);

            fire_particles[i].active = true;
            fire_particles[i].position = spaceship_position;
            fire_particles[i].max_life = 0.4f;
            fire_particles[i].life = fire_particles[i].max_life;
            fire_particles[i].velocity = Vector2Scale(fire_spread_direction , fire_speed);
            return;
        }
    }
}

#define max_bullet 32
#define bullet_speed 500.0f
#define bullet_lifetime 1.7f
#define bullet_frame_count 3
#define bullet_frame_speed 15.0f

typedef struct{
    Vector2 position;
    Vector2 velocity;
    float life;
    float max_life;
    float rotation;
    bool active;
    int current_frame;
    float frame_timer;
} Bullet;

Bullet bullets[max_bullet] = { 0 };
Texture2D bullet_texture;
float shooting_cooldown = 0.0f;
const float shoot_interval = 0.25f;

void InitBullets(void) {
    bullet_texture = LoadTexture("resources/laserBlue04.png");
}

void shoot_bullets(Vector2 spaceship_position , float spaceship_rotation){
    for(int i = 0; i < max_bullet; i++){
        if(!bullets[i].active){
            Vector2 forward = {sinf(spaceship_rotation * DEG2RAD) , -cosf(spaceship_rotation * DEG2RAD)};
            bullets[i].position = Vector2Add(spaceship_position , Vector2Scale(forward , 53.0f ));
            bullets[i].velocity = Vector2Scale(forward , bullet_speed);
            bullets[i].rotation = spaceship_rotation;
            bullets[i].life = bullet_lifetime;
            bullets[i].active = true;
            bullets[i].current_frame = 0;
            bullets[i].frame_timer = 0.0f;
            if (soundOn)
            {
            PlaySound(bullet_shoot);
            }
            return;
        }
    }
}

void draw_bullets(void){
    float frame_width = (float)bullet_texture.width / bullet_frame_count;
    float frame_height = (float)bullet_texture.height; 

    for(int i = 0 ; i < max_bullet ; i++){
        if(!bullets[i].active) continue;

        Rectangle source = { bullets[i].current_frame * frame_width , 0 , frame_width , frame_height};
        Rectangle destination = { bullets[i].position.x , bullets[i].position.y , frame_width , frame_height };
        Vector2 origin = { frame_width / 2.0f, frame_height / 2.0f };

        DrawTexturePro(bullet_texture , source , destination , origin , bullets[i].rotation , WHITE);   
    }

}

void unload_bullets(void){
    UnloadTexture(bullet_texture);
}

#define max_asteroids 64
#define asteroid_texture_count 3

typedef enum{
    asteroid_small = 4,
    asteroid_medium = 2,
    asteroid_large = 1,
} asteroid_size;


typedef struct{
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotation_speed;
    asteroid_size size;
    int texture_index;
    bool active;
    
} asteroid;

asteroid asteroids[max_asteroids] = {0};
Texture2D asteroid_texture[asteroid_texture_count];

float asteroid_radius(asteroid_size size){
    switch(size){
        case asteroid_large : return 65.0f;
        case asteroid_medium : return 50.0f;
        case asteroid_small : return 40.0f;
    }
    return 65.0f;
}

void InitAsteroid(void){
    asteroid_texture[0] = LoadTexture("resources/Asteroid_1.png");
    asteroid_texture[1] = LoadTexture("resources/Asteroid_3.png");
    asteroid_texture[2] = LoadTexture("resources/Asteroid_5.png");
}

void spawn_asteroid(asteroid_size size) {
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids[i].active) continue;

        Vector2 position;

        int edge = GetRandomValue(0 , 3);
        switch (edge) {
            case 0 : {
                position = (Vector2){GetRandomValue(0, screenwidth) , -20}; 
                break;
            }
            case 1 : {
                position = (Vector2){GetRandomValue(0,screenwidth) , screenlength + 20};
                break;
            }
            case 2 : {
                position = (Vector2){-20 , GetRandomValue(0 , screenlength)};
                break;
            }
            default : {
                position = (Vector2){screenwidth + 20 , GetRandomValue(0 , screenlength)};
                break;
            }
        }
    

    Vector2 center = {screenwidth / 2.0f , screenlength / 2.0f};
    Vector2 direction = Vector2Normalize(Vector2Subtract(center , position));
    float spread = (float)GetRandomValue(-45 , 45)* DEG2RAD;
    direction = Vector2Rotate(direction , spread);

    float speed;
    if(size == asteroid_large) speed = 60.0f;
    else if(size == asteroid_medium) speed = 90.0f;
    else speed = 150.0f;

    asteroids[i].position = position;
    asteroids[i].velocity = Vector2Scale(direction,speed);
    asteroids[i].rotation = (float)GetRandomValue(0 , 360);
    asteroids[i].rotation_speed = (float)GetRandomValue(-60 , 60);
    asteroids[i].size = size;
    asteroids[i].active = true;
    asteroids[i].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
    return;

    }
}

void update_asteroid(float dt){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids[i].active) continue;

        asteroids[i].position = Vector2Add(asteroids[i].position , Vector2Scale(asteroids[i].velocity, dt));
        asteroids[i].rotation = asteroids[i].rotation + asteroids[i].rotation_speed * dt;

        float asteroid_margin = 40.0f;

        if(asteroids[i].position.x < -asteroid_margin) asteroids[i].position.x = screenwidth + asteroid_margin;
        else if(asteroids[i].position.x > screenwidth + asteroid_margin ) asteroids[i].position.x = -asteroid_margin;

         if(asteroids[i].position.y < -asteroid_margin) asteroids[i].position.y = screenlength + asteroid_margin;
        else if(asteroids[i].position.y > screenlength + asteroid_margin ) asteroids[i].position.y = -asteroid_margin;

    } 
}

void draw_asteroid(void) {
    for(int i = 0; i < max_asteroids; i++){
        if(!asteroids[i].active) continue;

        Texture2D astro_tex = asteroid_texture[asteroids[i].texture_index];
        float radius = asteroid_radius(asteroids[i].size);
        float diameter = radius * 2.0f;

        Rectangle source = { 0 , 0 , (float)astro_tex.width , (float)astro_tex.height};
        Rectangle destination = {asteroids[i].position.x , asteroids[i].position.y ,diameter , diameter};
        Vector2 origin = {diameter / 2.0f , diameter / 2.0f};

        DrawTexturePro(astro_tex , source , destination , origin , asteroids[i].rotation , WHITE);

    }
}

void break_asteroid(int i){
    asteroid_size size = asteroids[i].size;
    Vector2 position = asteroids[i].position;
    asteroid_size new_size;
    
    asteroids[i].active = false;

    if(size == asteroid_small) return;

    if(size == asteroid_large){
         new_size = asteroid_medium;
    }
    else{
         new_size = asteroid_small;
    }

    for(int n = 0; n < 2 ; n++){
        for(int m = 0; m < max_asteroids ; m++ ){
            if(asteroids[m].active) continue;

            float angle = (float)GetRandomValue(0,360)* DEG2RAD;
            Vector2 direction = { cosf(angle) , sinf(angle)};
            float speed;
            if(new_size == asteroid_medium){
                speed = 90.0f;
            }
            else{
                speed = 120.0f;
            }

            asteroids[m].position = position;
            asteroids[m].velocity = Vector2Scale(direction , speed);
            asteroids[m].rotation = (float)GetRandomValue(0,360);
            asteroids[m].rotation_speed = (float)GetRandomValue(-60,60);
            asteroids[m].size = new_size;
            asteroids[m].active = true;
            asteroids[m].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
            break;
        }
    }

}

bool check_asteroid_hit(Vector2 bullet_position , int *hit_index){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids[i].active) continue;
        float radius = asteroid_radius(asteroids[i].size);
        if(CheckCollisionPointCircle(bullet_position, asteroids[i].position , radius)){
            *hit_index = i;
            return true;
        }
    }
    return false;
}

extern int score;

void update_bullets(float dt , Vector2 spaceship_position , float spaceship_rotation){
  shooting_cooldown = shooting_cooldown - dt;
    if(IsKeyDown (KEY_SPACE) && shooting_cooldown <= 0.0f){
        shoot_bullets(spaceship_position , spaceship_rotation); 
        shooting_cooldown = shoot_interval;
    }

    for(int i = 0; i < max_bullet ; i++){
        if(!bullets[i].active) continue;
         int hit_index;
        if(check_asteroid_hit(bullets[i].position , &hit_index)){
            if (soundOn)
            {
            PlaySound(astro_bul_col);
            }
               if(asteroids[hit_index].size == asteroid_large)

        score += 20;
    else if(asteroids[hit_index].size == asteroid_medium)
        score += 50;
    else if(asteroids[hit_index].size == asteroid_small)
        score += 100;
            break_asteroid(hit_index);
            bullets[i].active = false;
        }

        bullets[i].position = Vector2Add(bullets[i].position, Vector2Scale(bullets[i].velocity , dt));
        bullets[i].life = bullets[i].life - dt;

        bool off_screen = bullets[i].position.x < 0 || bullets[i].position.x > GetScreenWidth() || bullets[i].position.y < 0 || bullets[i].position.y > GetScreenHeight();

        if(bullets[i].life <= 0.0f || off_screen){
            bullets[i].active = false;
            continue;
        }

        if(bullet_frame_count > 1){
            bullets[i].frame_timer = bullets[i].frame_timer + dt;
            if(bullets[i].frame_timer >= 1.0f / bullet_frame_speed){
                bullets[i].frame_timer = 0.0f;
                bullets[i].current_frame = (bullets[i].current_frame + 1)% bullet_frame_count;
            }
        }
    }
   
}

bool check_any_active_asteroids(void){
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids[i].active) return true;
    }
    return false;
}

bool check_asteroid_spaceship_collision(Vector2 spaceship_position , float  spaceship_radius , int *hit_index){
    for(int i = 0 ; i < max_asteroids ; i++){
    if (!asteroids[i].active) continue ;
    float asteroid_r = asteroid_radius(asteroids[i].size);
    float total_rad = asteroid_r + spaceship_radius;

    if(Vector2Distance(spaceship_position , asteroids[i].position) <= total_rad){
        *hit_index = i;
        return true;
    }
    }
    return false;
}

void unload_asteroid(void){
    for(int i = 0; i < 3; i++){
    UnloadTexture(asteroid_texture[i]);
}
}

int score=0;
int LIVES = 5;
int game_over = 0;
bool scoreSaved = false;
float hit_cooldown = 0.0f;
char playerName[21] = "";
int nameLength = 0;

#define MAX_HALL_OF_FAME 5

char hallNames[MAX_HALL_OF_FAME][21];
int hallScores[MAX_HALL_OF_FAME] = {0};

void LoadHallOfFame(void)
{
    FILE *file = fopen("hall_of_fame.txt", "r");
    if (file == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_HALL_OF_FAME; i++)
    {
      fscanf(file, " %20[^|]|%d", hallNames[i], &hallScores[i]);
    }
    fclose(file);
}

void SaveHallOfFame(void)
{
    FILE *file = fopen("hall_of_fame.txt", "w");

    if (file == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_HALL_OF_FAME; i++)
    {
        if (hallScores[i] > 0)
        {
             fprintf(file, "%s|%d\n", hallNames[i], hallScores[i]);
        }
    }
    fclose(file);
}
void AddScoreToHallOfFame(void)
{
    int position = -1;
    for (int i = 0; i < MAX_HALL_OF_FAME; i++)
    {
        if (hallScores[i] == 0 || score > hallScores[i])
        {
            position = i;
            break;
        }
    }
    if (position == -1)
    {
        return;
    }
    for (int i = MAX_HALL_OF_FAME - 1; i > position; i--)
    {
        hallScores[i] = hallScores[i - 1];
        strcpy(hallNames[i], hallNames[i - 1]);
    }
    hallScores[position] = score;
    strcpy(hallNames[position], playerName);
    SaveHallOfFame();
}

int main(void)
{
   
    InitWindow( screenwidth, screenlength , "Spaceship");
    InitBullets();
    InitAsteroid();
    InitAudio();
    LoadHallOfFame();
    for(int o = 0; o < 4 ; o++){
        spawn_asteroid(asteroid_large);
    }
    
    SetExitKey(KEY_ESCAPE);
    //SetTargetFPS(60);

    Texture2D spaceship1_texture = LoadTexture("resources/Spaceship_3.png");
    Texture2D background1 = LoadTexture("resources/Starfield_08.png");
    Texture2D howToPlayBackground =LoadTexture("resources/how_to_play_background.png");
    Vector2 spaceship_position = (Vector2){screenwidth/2.0f , screenlength/2.0f};
    Vector2 spaceship_velocity = (Vector2){0.0f , 0.0f};
   
    float spaceship_rotation = 0.00f;
    float spaceship_rotation_speed = 180.00f;
    float spaceship_base_speed = 0.00f;
    float thrust = 600;
    float friction_per_second = 0.6f;
    
    GameState gameState = MENU;
    float menuShipTime = 0.0f;
float starTime = 0.0f;
float planetTime = 0.0f;

    while (!WindowShouldClose())
    {
  
        float dt = GetFrameTime();
if (gameState == NAME_ENTRY)
{
    int key = GetCharPressed();
    while (key > 0)
    {
        if (key >= 32 && key <= 125 && nameLength < 20)
        {
            playerName[nameLength] = (char)key;
            nameLength++;
            playerName[nameLength] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && nameLength > 0)
    {
        nameLength--;
        playerName[nameLength] = '\0';
    }
    
    if (IsKeyPressed(KEY_ENTER) && nameLength > 0)
{
    LIVES = 5;
    score = 0;
    game_over = 0;
    scoreSaved = false;

    spaceship_position = (Vector2){screenwidth / 2.0f, screenlength / 2.0f};
    spaceship_velocity = (Vector2){0.0f, 0.0f};
    spaceship_rotation = 0.0f;

    for (int i = 0; i < max_asteroids; i++)
    {
        asteroids[i].active = false;
    }

    for (int i = 0; i < 4; i++)
    {
        spawn_asteroid(asteroid_large);
    }

    StopMusicStream(menu_music);
    PlayMusicStream(bg_music);

    if (!soundOn)
    {
        PauseMusicStream(bg_music);
    }

    gameState = PLAYING;
}
}


        if (IsKeyPressed(KEY_S))
{
    soundOn = !soundOn;

    if (soundOn)
    {
        if (gameState == MENU)
        {
            ResumeMusicStream(menu_music);
        }
        else if (gameState == PLAYING)
        {
            PlayMusicStream(bg_music);
        }
        else if (gameState == HOW_TO_PLAY)
       {
            ResumeMusicStream(howToPlayMusic);
       }
    }
    else
    {
        PauseMusicStream(menu_music);
        PauseMusicStream(bg_music);
        PauseMusicStream(howToPlayMusic);
    }
}
        if (soundOn)
        {
        if (gameState == MENU)
{
    UpdateMusicStream(menu_music);
}
else if (gameState == PLAYING)
{
    UpdateMusicStream(bg_music);
}
 else if (gameState == HOW_TO_PLAY)
{
    UpdateMusicStream(howToPlayMusic);
}
        }
        if (hit_cooldown > 0.0f)
             hit_cooldown -= dt;
        if (gameState == MENU)
{
    menuShipTime += dt;
    starTime += dt;
    planetTime += dt;
}
        Vector2 spaceship_direction = (Vector2){cosf(DEG2RAD * (spaceship_rotation - 90)) , sinf(DEG2RAD * (spaceship_rotation - 90))};
    
        if (gameState == PLAYING)
{
        if(IsKeyDown(KEY_LEFT)){
            spaceship_rotation = spaceship_rotation - spaceship_rotation_speed * dt;
        }
        if(IsKeyDown(KEY_RIGHT)){
            spaceship_rotation = spaceship_rotation + spaceship_rotation_speed * dt;
        }
        if(IsKeyDown(KEY_UP)){
            spaceship_velocity = Vector2Add(spaceship_velocity , Vector2Scale(spaceship_direction , thrust*dt));
            for(int i = 0 ; i < 1 ; i++){
                spawn_particles(spaceship_position , spaceship_direction);
            }
        }
        spaceship_position = Vector2Add(spaceship_position , Vector2Scale(spaceship_velocity, dt));

        spaceship_velocity = Vector2Scale(spaceship_velocity , powf(friction_per_second , dt));
        
        
        if(spaceship_position.x > screenwidth + spaceship1_texture.width/2.0f) spaceship_position.x = 0;
        else if(spaceship_position.x <= 0 ) spaceship_position.x = screenwidth;

        if(spaceship_position.y > screenlength + spaceship1_texture.height/2.0f ) spaceship_position.y = 0;
        else if(spaceship_position.y <= 0) spaceship_position.y = screenlength;

         for(int i = 0 ; i < max_fire_particle ; i++){
           if(fire_particles[i].active ){
            fire_particles[i].position = Vector2Add(fire_particles[i].position , Vector2Scale(fire_particles[i].velocity,dt));
            fire_particles[i].life = fire_particles[i].life - dt;
            if(fire_particles[i].life <= 0){
                fire_particles[i].active = false;
            }
           }
        }

        
        update_asteroid(dt);
        update_bullets(dt , spaceship_position , spaceship_rotation);


        float spaceship_radius = spaceship1_texture.width/2.0f;
        int spaceship_hit_index;
        if(LIVES > 0 && hit_cooldown <= 0.0f && check_asteroid_spaceship_collision(spaceship_position, spaceship_radius , &spaceship_hit_index)){
            if (soundOn)
            {
            PlaySound(astro_ship_col);
            }
            LIVES--;
            hit_cooldown = 1.0f;
            spaceship_position = (Vector2) {screenwidth/2.0f , screenlength/2.0f};
            spaceship_velocity = (Vector2){0.00f , 0.00f};
        }

        if(LIVES == 0){
            game_over = 1;
            gameState = GAME_OVER;
            if (!scoreSaved)
             {
                AddScoreToHallOfFame();
                scoreSaved = true;
             }
        }

        while(!check_any_active_asteroids()){
            for(int o = 0; o < 4 ; o++){
            spawn_asteroid(asteroid_large);
          }
        }
    
    }

   if (gameState == MENU && IsKeyPressed(KEY_ENTER))
{
    StopMusicStream(menu_music);

    PlayMusicStream(bg_music);

    if (!soundOn)
    {
        PauseMusicStream(bg_music);
    }

    gameState = NAME_ENTRY;
}
if (gameState == MENU && IsKeyPressed(KEY_H))
{
    gameState = HALL_OF_FAME;
}
if (gameState == MENU && IsKeyPressed(KEY_P))
{
    gameState = HOW_TO_PLAY;

    if (soundOn)
    {
        PlayMusicStream(howToPlayMusic);
    }
}
if (gameState == HALL_OF_FAME && IsKeyPressed(KEY_M))
{
    gameState = MENU;
}
if (gameState == HOW_TO_PLAY && IsKeyPressed(KEY_M))
{
    StopMusicStream(howToPlayMusic);
    gameState = MENU;

    if (soundOn)
    {
        ResumeMusicStream(menu_music);
    }
}

if (gameState == GAME_OVER && IsKeyPressed(KEY_M))
{
    StopMusicStream(bg_music);
    PlayMusicStream(menu_music);

    if (!soundOn)
    {
        PauseMusicStream(menu_music);
    }

    gameState = MENU;
}
BeginDrawing();
if (gameState == HOW_TO_PLAY)
{
    float howToPlayGlow = (sinf(starTime * 2.5f) + 1.0f) / 2.0f;
    Rectangle sourceHowToPlay = {
        0,
        0,
        (float)howToPlayBackground.width,
        (float)howToPlayBackground.height
    };

    Rectangle destHowToPlay = {
        0,
        0,
        (float)screenwidth,
        (float)screenlength
    };

    Vector2 originHowToPlay = {0, 0};

    DrawTexturePro(
        howToPlayBackground,
        sourceHowToPlay,
        destHowToPlay,
        originHowToPlay,
        0.0f,
        WHITE
    );
    DrawRectangle(
    80,
    70,
    1340,
    760,
    (Color){5, 8, 25, 150}
);
DrawRectangleLines(
    72,
    62,
    1356,
    776,
    (Color){220, 170, 255, 45}
);
DrawRectangleLines(
    74,
    64,
    1352,
    772,
    (Color){225, 175, 255, 70}
);
DrawRectangleLines(
    76,
    66,
    1348,
    768,
    (Color){230, 180, 255, 100}
);
DrawRectangleLines(
    78,
    68,
    1344,
    764,
    (Color){235, 185, 255, 145}
);
DrawRectangleLines(
    80,
    70,
    1340,
    760,
    (Color){240, 195, 255, 255}
);
unsigned char glowAlpha =(unsigned char)(50 + howToPlayGlow * 205);
DrawText(
    "HOW TO PLAY",
    556,
    90,
    60,
    (Color){255, 20, 180, 120}
);
DrawText(
    "HOW TO PLAY",
    558,
    92,
    60,
    (Color){255, 20, 180, 120}
);
DrawText(
    "HOW TO PLAY",
    560,
    94,
    60,
    (Color){255, 80, 200, 255}
);
DrawText(
    "CONTROLS",
    176,
    171,
    55,
    (Color){255, 220, 80, 150}
);
DrawText(
    "CONTROLS",
    180,
    175,
    55,
    (Color){255, 235, 100, 255}
);
DrawText(
    "UP ARROW",
    180,
    270,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "UP ARROW",
    176,
    266,
    36,
    WHITE
);
DrawText(
    "THRUST",
    650,
    270,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "THRUST",
    646,
    266,
    36,
    WHITE
);
DrawText(
    "LEFT ARROW",
    180,
    350,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "LEFT ARROW",
    176,
    346,
    36,
    WHITE
);
DrawText(
    "ROTATE LEFT",
    650,
    350,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "ROTATE LEFT",
    646,
    346,
    36,
    WHITE
);
DrawText(
    "RIGHT ARROW",
    180,
    430,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "RIGHT ARROW",
    176,
    426,
    36,
    WHITE
);
DrawText(
    "ROTATE RIGHT",
    650,
    430,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "ROTATE RIGHT",
    646,
    426,
    36,
    WHITE
);
DrawText(
    "SPACE",
    180,
    510,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "SPACE",
    176,
    506,
    36,
    WHITE
);
DrawText(
    "SHOOT",
    650,
    510,
    36,
    (Color){255, 100, 190, 120}
);
DrawText(
    "SHOOT",
    646,
    506,
    36,
    WHITE
);
DrawCircle(
    1120,
    580,
    28,
    (Color){220, 230, 245, 255}
);
DrawCircle(
    1120,
    580,
    20,
    (Color){30, 45, 75, 255}
);
DrawCircle(
    1114,
    574,
    5,
    (Color){180, 220, 255, 180}
);
DrawLine(
    1120,
    557,
    1126,
    545,
    (Color){180, 190, 210, 255}
);
DrawCircle(
    1128,
    542,
    4,
    (Color){255, 100, 190, 255}
);
DrawRectangle(
    1082,
    622,
    18,
    34,
    (Color){110, 20, 35, 255}
);
DrawCircle(
    1091,
    622,
    9,
    (Color){110, 20, 35, 255}
);
DrawCircle(
    1091,
    656,
    9,
    (Color){110, 20, 35, 255}
);
DrawRectangle(
    1086,
    627,
    10,
    24,
    (Color){170, 35, 50, 255}
);
DrawRectangle(
    1098,
    608,
    44,
    55,
    (Color){220, 230, 245, 255}
);
DrawRectangle(
    1110,
    625,
    20,
    14,
    (Color){30, 45, 75, 255}
);
DrawCircle(
    1115,
    632,
    2,
    (Color){255, 100, 190, 255}
);
DrawCircle(
    1124,
    632,
    2,
    (Color){100, 220, 255, 255}
);
DrawRectangle(
    1084,
    615,
    14,
    35,
    (Color){220, 230, 245, 255}
);
DrawRectangle(
    1142,
    615,
    14,
    35,
    (Color){220, 230, 245, 255}
);
DrawRectangle(
    1085,
    644,
    14,
    13,
    (Color){180, 190, 210, 255}
);
DrawRectangle(
    1141,
    644,
    14,
    13,
    (Color){180, 190, 210, 255}
);
DrawRectangle(
    1100,
    663,
    16,
    22,
    (Color){220, 230, 245, 255}
);
DrawRectangle(
    1124,
    663,
    16,
    22,
    (Color){220, 230, 245, 255}
);
DrawRectangle(
    1097,
    685,
    22,
    10,
    (Color){120, 130, 155, 255}
);
DrawRectangle(
    1121,
    685,
    22,
    10,
    (Color){120, 130, 155, 255}
);
DrawLine(
    1268,
    503,
    1235,
    536,
    (Color){170, 210, 255, 100}
);
DrawLine(
    1265,
    500,
    1228,
    537,
    (Color){255, 180, 230, 80}
);
DrawLine(
    1262,
    497,
    1240,
    519,
    (Color){255, 235, 190, 100}
);
DrawCircle(
    1272,
    496,
    8,
    (Color){255, 225, 150, 100}
);
DrawCircle(
    1272,
    496,
    5,
    (Color){255, 245, 210, 255}
);
DrawText(
    "MAY THE STARS GUIDE YOUR JOURNEY,",
    930,
    700,
    24,
    (Color){80, 170, 255, 120}
);
DrawText(
    "MAY THE STARS GUIDE YOUR JOURNEY,",
    926,
    696,
    24,
    (Color){170, 220, 255, 255}
);
DrawText(
    "SPACE EXPLORER!",
    1080,
    735,
    28,
    (Color){80, 170, 255, 120}
);
DrawText(
    "SPACE EXPLORER!",
    1076,
    731,
    28,
    (Color){170, 220, 255, 255}
);
DrawText(
    "PRESS M TO RETURN TO MENU",
    550,
    790,
    24,
    (Color){120, 255, 170, 255}
);
}
else
{
    Rectangle source1 = {
        0,
        0,
        (float)background1.width,
        (float)background1.height
    };
    Rectangle dest1 = {
        0,
        0,
        (float)screenwidth,
        (float)screenlength
    };
    Vector2 origin1 = {0, 0};
    DrawTexturePro(
        background1,
        source1,
        dest1,
        origin1,
        0.0f,
        WHITE
    );
}
        if (gameState == MENU)
{   DrawRectangle(0, 0, screenwidth, screenlength, (Color){5, 8, 25, 255});
   
float twinkle = (sinf(starTime * 2.5f) + 1.0f) / 2.0f;
float twinkle2 = (sinf(starTime * 4.0f + 2.0f) + 1.0f) / 2.0f;

DrawCircle(100, 100, 2, WHITE);
DrawCircle(250, 180, 2, WHITE);
DrawCircle(400, 80, 1, WHITE);
DrawCircle(600, 150, 2, WHITE);
DrawCircle(850, 100, 2, WHITE);
DrawCircle(1100, 180, 2, WHITE);
DrawCircle(1250, 80, 1, WHITE);
DrawCircle(1350, 300, 2, WHITE);
DrawCircle(150, 500, 2, WHITE);
DrawCircle(300, 650, 1, WHITE);
DrawCircle(1000, 600, 2, WHITE);
DrawCircle(1200, 500, 1, WHITE);


DrawCircle(50, 300, 1, WHITE);
DrawCircle(180, 350, 2, WHITE);
DrawCircle(350, 250, 1, WHITE);
DrawCircle(500, 100, 1, WHITE);
DrawCircle(750, 250, 2, WHITE);
DrawCircle(950, 350, 1, WHITE);
DrawCircle(1150, 400, 2, WHITE);
DrawCircle(1300, 550, 1, WHITE);
DrawCircle(1400, 700, 2, WHITE);
DrawCircle(800, 700, 1, WHITE);
DrawCircle(450, 750, 2, WHITE);
DrawCircle(100, 750, 1, WHITE);


float glow1 = 3.0f + twinkle * 3.0f;

DrawCircle(
    200,
    120,
    glow1 * 2,
    (Color){100, 160, 255, 35}
);
DrawCircle(
    200,
    120,
    glow1,
    (Color){180, 220, 255, 180}
);

float glow2 = 2.0f + twinkle2 * 4.0f;

DrawCircle(
    900,
    180,
    glow2 * 2,
    (Color){100, 160, 255, 35}
);
DrawCircle(
    900,
    180,
    glow2,
    (Color){220, 240, 255, 200}
);

float sparkle = 4.0f + twinkle * 5.0f;

DrawCircle(
    700,
    80,
    sparkle,
    (Color){150, 200, 255, 100}
);
DrawLine(
    700 - sparkle * 2,
    80,
    700 + sparkle * 2,
    80,
    (Color){220, 240, 255, 180}
);
DrawLine(
    700,
    80 - sparkle * 2,
    700,
    80 + sparkle * 2,
    (Color){220, 240, 255, 180}
);
       DrawCircle(1100, 600, 250, (Color){30,60,150,120});
       DrawCircle(1100, 600, 220, (Color){20, 40, 100, 255});
       DrawCircle(1060, 550, 150, (Color){30,55,120,120});     
DrawCircleLines(
    1100,
    600,
    225,
    (Color){80, 160, 255, 180}
);
DrawCircleLines(
    1100,
    600,
    230,
    (Color){60, 130, 255, 80}
);
DrawCircle(
    1160,
    630,
    185,
    (Color){3, 8, 30, 120}
);
DrawCircle(
    1190,
    650,
    150,
    (Color){2, 5, 20, 100}
);
DrawCircle(
    1030,
    500,
    90,
    (Color){100, 170, 255, 35}
);
DrawCircle(
    1030,
    500,
    45,
    (Color){35, 75, 150, 120}
);
DrawCircle(
    1080,
    680,
    55,
    (Color){15, 35, 90, 140}
);
DrawCircle(
    1190,
    530,
    35,
    (Color){40, 80, 155, 100}
);
DrawCircle(
    1250,
    650,
    50,
    (Color){10, 25, 70, 130}
);
DrawCircle(
    1000,
    620,
    25,
    (Color){45, 90, 170, 100}
);
DrawCircleLines(
    1100,
    600,
    220,
    (Color){120, 200, 255, 180}
);
DrawCircleLines(
    1100,
    600,
    216,
    (Color){80, 160, 255, 100}
);
     int titleWidth = MeasureText("WELCOME TO THE ULKA GAME", 50);

 DrawText(
    "WELCOME TO THE ULKA GAME",
    (screenwidth - titleWidth) / 2 + 7,
    157,
    50,
    (Color){5, 20, 70, 255}
);
DrawText(
    "WELCOME TO THE ULKA GAME",
    (screenwidth - titleWidth) / 2 + 3,
    153,
    50,
    (Color){50, 120, 220, 255}
);
DrawText(
    "WELCOME TO THE ULKA GAME",
    (screenwidth - titleWidth) / 2,
    150,
    50,
    (Color){220, 240, 255, 255}
);

 Rectangle startButton = {
    200,
    650,
    280,
    65
};

Rectangle hallOfFameButton = {
    520,
    650,
    280,
    65
};

Rectangle howToPlayButton = {
    200,
    730,
    280,
    65
};

Rectangle soundButton = {
    520,
    730,
    280,
    65
};

float buttonPulse = (sinf(menuShipTime * 3.0f) + 1.0f) / 2.0f;
DrawRectangle(
    startButton.x - 18,
    startButton.y - 18,
    startButton.width + 36,
    startButton.height + 36,
    (Color){30, 80, 180, (unsigned char)(35 + buttonPulse * 45)}
);
DrawRectangle(
    startButton.x - 9,
    startButton.y - 9,
    startButton.width + 18,
    startButton.height + 18,
    (Color){40, 100, 240, (unsigned char)(50 + buttonPulse * 60)}
);
DrawRectangleRec(
    startButton,
    (Color){15, 45, 100, 255}
);
DrawRectangleLinesEx(
    startButton,
    3,
    (Color){100, 180, 255, (unsigned char)(180 + buttonPulse * 75)}
);

int startWidth = MeasureText("PRESS ENTER TO START", 20);
DrawText(
    "PRESS ENTER TO START",
    startButton.x + (startButton.width - startWidth) / 2,
    startButton.y + 24,
    20,
    WHITE
);

DrawRectangle(
    hallOfFameButton.x - 18,
    hallOfFameButton.y - 18,
    hallOfFameButton.width + 36,
    hallOfFameButton.height + 36,
    (Color){30, 80, 180, (unsigned char)(35 + buttonPulse * 45)}
);

DrawRectangle(
    hallOfFameButton.x - 9,
    hallOfFameButton.y - 9,
    hallOfFameButton.width + 18,
    hallOfFameButton.height + 18,
    (Color){40, 100, 240, (unsigned char)(50 + buttonPulse * 60)}
);

DrawRectangleRec(
    hallOfFameButton,
    (Color){15, 45, 100, 255}
);

DrawRectangleLinesEx(
    hallOfFameButton,
    3,
    (Color){100, 180, 255, (unsigned char)(180 + buttonPulse * 75)}
);

int hallWidth = MeasureText("HALL OF FAME", 20);

DrawText(
    "HALL OF FAME [H]",
    hallOfFameButton.x + (hallOfFameButton.width - hallWidth) / 2,
    hallOfFameButton.y + 20,
    20,
    WHITE
);

DrawRectangle(
    howToPlayButton.x - 18,
    howToPlayButton.y - 18,
    howToPlayButton.width + 36,
    howToPlayButton.height + 36,
    (Color){30, 80, 180, (unsigned char)(35 + buttonPulse * 45)}
);
DrawRectangle(
    howToPlayButton.x - 9,
    howToPlayButton.y - 9,
    howToPlayButton.width + 18,
    howToPlayButton.height + 18,
    (Color){40, 100, 240, (unsigned char)(50 + buttonPulse * 60)}
);
DrawRectangleRec(
    howToPlayButton,
    (Color){15, 45, 100, 255}
);
DrawRectangleLinesEx(
    howToPlayButton,
    3,
    (Color){100, 180, 255, (unsigned char)(180 + buttonPulse * 75)}

);

int howToPlayWidth = MeasureText("HOW TO PLAY",20);
DrawText(
    "HOW TO PLAY [P]",
    howToPlayButton.x + (howToPlayButton.width - howToPlayWidth) / 2,
    howToPlayButton.y + 20,
    20,
    WHITE
);

DrawRectangle(
    soundButton.x - 18,
    soundButton.y - 18,
    soundButton.width + 36,
    soundButton.height + 36,
    (Color){30, 80, 180, (unsigned char)(35 + buttonPulse * 45)}
);
DrawRectangle(
    soundButton.x - 9,
    soundButton.y - 9,
    soundButton.width + 18,
    soundButton.height + 18,
    (Color){40, 100, 240, (unsigned char)(50 + buttonPulse * 60)}
);
DrawRectangleRec(
    soundButton,
    (Color){15, 45, 100, 255}
);
DrawRectangleLinesEx(
    soundButton,
    3,
    (Color){100, 180, 255, (unsigned char)(180 + buttonPulse * 75)}
);
const char *soundText;
if (soundOn)
{
    soundText = "SOUND: ON [S to OFF]";
}
else
{
    soundText = "SOUND: OFF [S to ON]";
}

int soundTextWidth = MeasureText(soundText, 20);
DrawText(
    soundText,
    soundButton.x + (soundButton.width - soundTextWidth) / 2,
    soundButton.y + 20,
    20,
    WHITE
);

   Rectangle menuShipSource = {
    0,
    0,
    (float)spaceship1_texture.width,
    (float)spaceship1_texture.height
};
float shipY = 400 + sinf(menuShipTime * 2.0f) * 8.0f;
DrawCircle(
    600,
    shipY,
    110,
    (Color){40, 80, 180, 40}
);
DrawCircle(
    600,
    shipY,
    70,
    (Color){60, 100, 220, 35}
);
float enginePulse = (sinf(menuShipTime * 8.0f) + 1.0f) / 2.0f;
float engineSize = 8.0f + enginePulse * 6.0f;
float engineLength = 25.0f + enginePulse * 15.0f;
DrawCircle(
    600,
    shipY + 45,
    engineSize * 2.5f,
    (Color){30, 100, 255, 40}
);
DrawCircle(
    600,
    shipY + 45,
    engineSize,
    (Color){100, 200, 255, 180}
);
DrawTriangle(
    (Vector2){600 - engineSize, shipY + 45},
    (Vector2){600 + engineSize, shipY + 45},
    (Vector2){600, shipY + 45 + engineLength},
    (Color){60, 150, 255, 100}
);

Vector2 menuShipPosition = {
    600,
   shipY
};

Vector2 menuShipOrigin = {
    spaceship1_texture.width / 2.0f,
    spaceship1_texture.height / 2.0f
};

DrawTexturePro(
    spaceship1_texture,
    menuShipSource,
    (Rectangle){
        menuShipPosition.x,
        menuShipPosition.y,
        (float)spaceship1_texture.width,
        (float)spaceship1_texture.height
    },
    menuShipOrigin,
    0.0f,
    WHITE
);
}   
if (gameState == NAME_ENTRY)
{
    DrawRectangle(
        0,
        0,
        screenwidth,
        screenlength,
        (Color){5, 8, 25, 255}
    );

    const char *nameTitle = "ENTER YOUR NAME";
    int nameTitleWidth = MeasureText(nameTitle, 50);

    DrawText(
        nameTitle,
        (screenwidth - nameTitleWidth) / 2,
        220,
        50,
        (Color){220, 240, 255, 255}
    );
    Rectangle nameBox = {
        450,
        330,
        600,
        80
    };
    DrawRectangleRec(
        nameBox,
        (Color){15, 45, 100, 255}
    );
    DrawRectangleLinesEx(
        nameBox,
        3,
        (Color){100, 180, 255, 255}
    );
    int nameWidth = MeasureText(playerName, 30);

    DrawText(
        playerName,
        nameBox.x + (nameBox.width - nameWidth) / 2,
        nameBox.y + 23,
        30,
        WHITE
    );
    if ((int)(GetTime() * 2) % 2 == 0)
    {
        int cursorX = nameBox.x + (nameBox.width + nameWidth) / 2;

        DrawRectangle(
            cursorX + 3,
            nameBox.y + 18,
            2,
            40,
            WHITE
        );
    }
    const char *nameInstruction = "TYPE YOUR NAME  -  PRESS ENTER TO CONTINUE";
    int instructionWidth = MeasureText(nameInstruction, 20);

    DrawText(
        nameInstruction,
        (screenwidth - instructionWidth) / 2,
        450,
        20,
        (Color){180, 220, 255, 255}
    );
}

if (gameState == PLAYING)
{
       for(int i = 0 ; i < max_fire_particle ; i++){
        if(fire_particles[i].active){
            float time = fire_particles[i].life / fire_particles[i].max_life;
            Color fire = { 200 , (unsigned char)(60*time) , 0 , (unsigned char)(160*time)};
            float flicker = 1.0f + 0.25f * sinf(fire_particles[i].life * 50.0f + i);
            float radius = 3.0f*time*flicker;
            DrawCircleV(fire_particles[i].position , radius , fire);
        }
       }

       Rectangle dest2 = {spaceship_position.x , spaceship_position.y ,(float)spaceship1_texture.width , (float)spaceship1_texture.height };
        Rectangle source2 = {0 , 0 , (float)spaceship1_texture.width , (float)spaceship1_texture.height};
        Vector2 origin = {spaceship1_texture.width/2.0f , spaceship1_texture.height/2.0f};
        DrawTexturePro(spaceship1_texture , source2 , dest2 , origin , spaceship_rotation , WHITE );  

     DrawText(TextFormat("SCORE: %d", score), 20, 20, 30, WHITE);
     DrawText(TextFormat("LIVES: %d", LIVES), 20, 55, 30, WHITE);
     DrawText(TextFormat("PLAYER: %s", playerName), 20, 90, 25, WHITE);
     if (soundOn)
{
    DrawText(
        "SOUND: ON",
        screenwidth - 180,
        25,
        20,
        (Color){180, 220, 255, 255}
    );
}
else
{
    DrawText(
        "SOUND: OFF",
        screenwidth - 190,
        25,
        20,
        (Color){180, 180, 180, 255}
    );
}
        draw_bullets();
        draw_asteroid(); 
    }
    if (gameState == GAME_OVER)
{
    DrawText("GAME OVER", screenwidth/2.0f - 250, screenlength/2.0f - 50, 80, WHITE);
    DrawText("PRESS M TO RETURN TO MENU", screenwidth/2.0f - 210, screenlength/2.0f + 50, 30, WHITE);
}
if (gameState == HALL_OF_FAME)
{
    DrawRectangle(
        0,
        0,
        screenwidth,
        screenlength,
        (Color){5, 8, 25, 255}
    );
    DrawText(
        "HALL OF FAME",
        screenwidth / 2 - 200,
        150,
        60,
        (Color){220, 240, 255, 255}
    );
    for (int i = 0; i < MAX_HALL_OF_FAME; i++)
    {
        if (hallScores[i] > 0)
        {
            DrawText(
                TextFormat("%d. %s", i + 1, hallNames[i]),
                450,
                280 + i * 70,
                30,
                WHITE
            );
            DrawText(
                TextFormat("%d", hallScores[i]),
                950,
                280 + i * 70,
                30,
                WHITE
            );
        }
    }
    DrawText(
        "PRESS M TO RETURN TO MENU",
        screenwidth / 2 - 180,
        700,
        25,
        (Color){180, 220, 255, 255}
    );
}
        DrawFPS(0 , 0);
        EndDrawing();
    }
    unload_asteroid();
    unload_bullets();
    unload_audio();
    CloseWindow(); 

    return 0;
}
