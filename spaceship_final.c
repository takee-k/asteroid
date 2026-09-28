#include "raylib.h"
#include "raymath.h"
#include<stdio.h>
#include<stdlib.h>

#define screenwidth  1500
#define screenlength  900


float level_timer = 0.0f;
float level_timer_for_story = 0.0f;
const float story_timer = 10.0f;
const float level_1_timer = 10.0f;
const float level_2_timer = 10.0f;
const float level_3_timer = 180.0f;

int  wave = 0;
int wave_2 = 0;
int wave_3 = 0;


typedef enum
{
    MENU,
    STORY_1,
    PLAYING,
    STORY_2,
    PLAYING_2,
    STORY_3,
    PLAYING_3,
    STORY_4,
    GAME_OVER
} GameState;

Music menu_music;
Music bg_music;
Music bg_music2;
Music bg_music3;
Music story_line;
Sound bullet_shoot;
Sound astro_bul_col;
Sound steel_bul_col;
Sound stone_bul_col;
Sound astro_ship_col;
Sound enemy_coming_hehe;
Sound enemy_dying;



void InitAudio(void){
    InitAudioDevice();
    menu_music = LoadMusicStream("audio/tokyorifft-interstellar-374344.mp3");
    bg_music = LoadMusicStream("audio/atlasaudio-ambient-astronomy-511860.mp3");
    bg_music2 = LoadMusicStream("audio/delosound-cinematic-space-background-263169.mp3");
    bg_music3 = LoadMusicStream("audio/SignOfTheTimesHS.mp3");
    story_line = LoadMusicStream("audio/mondamusic-space-589110.mp3");

    bullet_shoot = LoadSound("audio/freesound_community-fire-88783.mp3");
    astro_bul_col = LoadSound("audio/dragon-studio-explosion-sound-effect-425455.mp3");
    astro_ship_col = LoadSound("audio/finntastico-asteroid-hitting-something-152511.mp3");
    steel_bul_col = LoadSound("audio/dragon-studio-sword-breaking-sound-effect-393840.mp3");
    stone_bul_col = LoadSound("audio/dragon-studio-boulder-impact-487673.mp3");

    enemy_coming_hehe = LoadSound("audio/dragon-studio-alien-song-323613.mp3");
    enemy_dying = LoadSound("audio/scream.wav");
    

    PlayMusicStream(menu_music);
}

void unload_audio(void){
    UnloadMusicStream(menu_music);
    UnloadMusicStream( bg_music);
    UnloadSound( bullet_shoot);
    UnloadSound(astro_bul_col);
    UnloadSound(astro_ship_col);
    UnloadSound(enemy_coming_hehe);
    UnloadMusicStream(story_line);
    UnloadSound(steel_bul_col);
    UnloadMusicStream(bg_music2);
    UnloadMusicStream(bg_music3);
    UnloadSound(enemy_dying);

    CloseAudioDevice();

}




typedef struct fire_particle{
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

typedef struct Bullet{
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
            PlaySound(bullet_shoot);
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


typedef struct asteroid {
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

            float angle = (float)GetRandomValue(0,360)*DEG2RAD;
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



typedef enum{
    asteroid_small_1 = 4,
    asteroid_medium_1 = 2,
    asteroid_large_1 = 1,
} asteroid_size_1;


typedef struct asteroid_1 {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotation_speed;
    asteroid_size size;
    int texture_index;
    bool active;
    
} asteroid_1;

asteroid asteroids_1[max_asteroids] = {0};
Texture2D asteroid_texture_1[asteroid_texture_count];

float asteroid_radius_1(asteroid_size size){
    switch(size){
        case asteroid_large : return 65.0f;
        case asteroid_medium : return 50.0f;
        case asteroid_small : return 40.0f;
    }
    return 65.0f;
}

void InitAsteroid_1(void){
    asteroid_texture_1[0] = LoadTexture("resources/Metal_Spike_Ball_1.png");
    asteroid_texture_1[1] = LoadTexture("resources/Spike_Ball_2.png");
    asteroid_texture_1[2] = LoadTexture("resources/Shrinker.png");
}

void spawn_asteroid_1(asteroid_size size) {
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids_1[i].active) continue;

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
    if(size == asteroid_large_1) speed = 60.0f;
    else if(size == asteroid_medium_1) speed = 90.0f;
    else speed = 150.0f;

    asteroids_1[i].position = position;
    asteroids_1[i].velocity = Vector2Scale(direction,speed);
    asteroids_1[i].rotation = (float)GetRandomValue(0 , 360);
    asteroids_1[i].rotation_speed = (float)GetRandomValue(-60 , 60);
    asteroids_1[i].size = size;
    asteroids_1[i].active = true;
    asteroids_1[i].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
    return;


    }
}



void update_asteroid_1(float dt){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids_1[i].active) continue;

        asteroids_1[i].position = Vector2Add(asteroids_1[i].position , Vector2Scale(asteroids_1[i].velocity, dt));
        asteroids_1[i].rotation = asteroids_1[i].rotation + asteroids_1[i].rotation_speed * dt;

        float asteroid_margin = 40.0f;

        if(asteroids_1[i].position.x < -asteroid_margin) asteroids_1[i].position.x = screenwidth + asteroid_margin;
        else if(asteroids_1[i].position.x > screenwidth + asteroid_margin ) asteroids_1[i].position.x = -asteroid_margin;

         if(asteroids_1[i].position.y < -asteroid_margin) asteroids_1[i].position.y = screenlength + asteroid_margin;
        else if(asteroids_1[i].position.y > screenlength + asteroid_margin ) asteroids_1[i].position.y = -asteroid_margin;

    } 
}

void draw_asteroid_1(void) {
    for(int i = 0; i < max_asteroids; i++){
        if(!asteroids_1[i].active) continue;

        Texture2D astro_tex = asteroid_texture_1[asteroids_1[i].texture_index];
        float radius = asteroid_radius(asteroids_1[i].size);
        float diameter = radius * 2.0f;

        Rectangle source = { 0 , 0 , (float)astro_tex.width , (float)astro_tex.height};
        Rectangle destination = {asteroids_1[i].position.x , asteroids_1[i].position.y ,diameter , diameter};
        Vector2 origin = {diameter / 2.0f , diameter / 2.0f};

        DrawTexturePro(astro_tex , source , destination , origin , asteroids_1[i].rotation , WHITE);

    }
}





void break_asteroid_1(int i){
    asteroid_size size = asteroids_1[i].size;
    Vector2 position = asteroids_1[i].position;
    asteroid_size new_size;
    

    asteroids_1[i].active = false;

    if(size == asteroid_small_1) return;

    if(size == asteroid_large_1){
         new_size = asteroid_medium_1;
    }
    else{
         new_size = asteroid_small_1;
    }

    for(int n = 0; n < 2 ; n++){
        for(int m = 0; m < max_asteroids ; m++ ){
            if(asteroids_1[m].active) continue;

            float angle = (float)GetRandomValue(0,360)*DEG2RAD;
            Vector2 direction = { cosf(angle) , sinf(angle)};
            float speed;
            if(new_size == asteroid_medium_1){
                speed = 90.0f;
            }
            else{
                speed = 120.0f;
            }

            asteroids_1[m].position = position;
            asteroids_1[m].velocity = Vector2Scale(direction , speed);
            asteroids_1[m].rotation = (float)GetRandomValue(0,360);
            asteroids_1[m].rotation_speed = (float)GetRandomValue(-60,60);
            asteroids_1[m].size = new_size;
            asteroids_1[m].active = true;
            asteroids_1[m].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
            break;
        }
    }


}


bool check_asteroid_hit_1(Vector2 bullet_position , int *hit_index){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids_1[i].active) continue;
        float radius = asteroid_radius_1(asteroids_1[i].size);
        if(CheckCollisionPointCircle(bullet_position, asteroids_1[i].position , radius)){
            *hit_index = i;
            return true;
        }
    }
    return false;
}




typedef enum{
    asteroid_small_2 = 4,
    asteroid_medium_2 = 2,
    asteroid_large_2 = 1,
} asteroid_size_2;


typedef struct asteroid_2 {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotation_speed;
    asteroid_size size;
    int texture_index;
    bool active;
    
} asteroid_2;

asteroid asteroids_2[max_asteroids] = {0};
Texture2D asteroid_texture_2[asteroid_texture_count];

float asteroid_radius_2(asteroid_size size){
    switch(size){
        case asteroid_large_2 : return 75.0f;
        case asteroid_medium_2 : return 60.0f;
        case asteroid_small_2 : return 50.0f;
    }
    return 65.0f;
}

void InitAsteroid_2(void){
    asteroid_texture_2[0] = LoadTexture("resources/stone001.png");
    asteroid_texture_2[1] = LoadTexture("resources/stone003.png");
    asteroid_texture_2[2] = LoadTexture("resources/stone004.png");
}

void spawn_asteroid_2(asteroid_size size) {
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids_2[i].active) continue;

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
    if(size == asteroid_large_2) speed = 60.0f;
    else if(size == asteroid_medium_2) speed = 90.0f;
    else speed = 150.0f;

    asteroids_2[i].position = position;
    asteroids_2[i].velocity = Vector2Scale(direction,speed);
    asteroids_2[i].rotation = (float)GetRandomValue(0 , 360);
    asteroids_2[i].rotation_speed = (float)GetRandomValue(-60 , 60);
    asteroids_2[i].size = size;
    asteroids_2[i].active = true;
    asteroids_2[i].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
    return;


    }
}



void update_asteroid_2(float dt){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids_2[i].active) continue;

        asteroids_2[i].position = Vector2Add(asteroids_2[i].position , Vector2Scale(asteroids_2[i].velocity, dt));
        asteroids_2[i].rotation = asteroids_2[i].rotation + asteroids_2[i].rotation_speed * dt;

        float asteroid_margin = 40.0f;

        if(asteroids_2[i].position.x < -asteroid_margin) asteroids_2[i].position.x = screenwidth + asteroid_margin;
        else if(asteroids_2[i].position.x > screenwidth + asteroid_margin ) asteroids_2[i].position.x = -asteroid_margin;

         if(asteroids_2[i].position.y < -asteroid_margin) asteroids_2[i].position.y = screenlength + asteroid_margin;
        else if(asteroids_2[i].position.y > screenlength + asteroid_margin ) asteroids_2[i].position.y = -asteroid_margin;

    } 
}

void draw_asteroid_2(void) {
    for(int i = 0; i < max_asteroids; i++){
        if(!asteroids_2[i].active) continue;

        Texture2D astro_tex = asteroid_texture_2[asteroids_2[i].texture_index];
        float radius = asteroid_radius_2(asteroids_2[i].size);
        float diameter = radius * 2.0f;

        Rectangle source = { 0 , 0 , (float)astro_tex.width , (float)astro_tex.height};
        Rectangle destination = {asteroids_2[i].position.x , asteroids_2[i].position.y ,diameter , diameter};
        Vector2 origin = {diameter / 2.0f , diameter / 2.0f};

        DrawTexturePro(astro_tex , source , destination , origin , asteroids_2[i].rotation , WHITE);

    }
}



void break_asteroid_2(int i){
    asteroid_size size = asteroids_2[i].size;
    Vector2 position = asteroids_2[i].position;
    asteroid_size new_size;
    

    asteroids_2[i].active = false;

    if(size == asteroid_small_2) return;

    if(size == asteroid_large_2){
         new_size = asteroid_medium_2;
    }
    else{
         new_size = asteroid_small_2;
    }

    for(int n = 0; n < 2 ; n++){
        for(int m = 0; m < max_asteroids ; m++ ){
            if(asteroids_2[m].active) continue;

            float angle = (float)GetRandomValue(0,360)*DEG2RAD;
            Vector2 direction = { cosf(angle) , sinf(angle)};
            float speed;
            if(new_size == asteroid_medium_2){
                speed = 90.0f;
            }
            else{
                speed = 120.0f;
            }

            asteroids_2[m].position = position;
            asteroids_2[m].velocity = Vector2Scale(direction , speed);
            asteroids_2[m].rotation = (float)GetRandomValue(0,360);
            asteroids_2[m].rotation_speed = (float)GetRandomValue(-60,60);
            asteroids_2[m].size = new_size;
            asteroids_2[m].active = true;
            asteroids_2[m].texture_index = GetRandomValue(0 , asteroid_texture_count - 1);
            break;
        }
    }


}


bool check_asteroid_hit_2(Vector2 bullet_position , int *hit_index){
    for(int i = 0; i < max_asteroids ; i++){
        if(!asteroids_2[i].active) continue;
        float radius = asteroid_radius_2(asteroids_2[i].size);
        if(CheckCollisionPointCircle(bullet_position, asteroids_2[i].position , radius)){
            *hit_index = i;
            return true;
        }
    }
    return false;
}


extern int score;



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

bool check_any_active_asteroids_1(void){
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids_1[i].active) return true;
    }
    return false;
}


bool check_asteroid_spaceship_collision_1(Vector2 spaceship_position , float  spaceship_radius , int *hit_index){
    for(int i = 0 ; i < max_asteroids ; i++){
    if (!asteroids_1[i].active) continue ;
    float asteroid_r = asteroid_radius(asteroids_1[i].size);
    float total_rad = asteroid_r + spaceship_radius;

    if(Vector2Distance(spaceship_position , asteroids_1[i].position) <= total_rad){
        *hit_index = i;
        return true;
    }
    }
    return false;
}



void unload_asteroid_1(void){
    for(int i = 0; i < 3; i++){
    UnloadTexture(asteroid_texture_1[i]);
}
}


bool check_any_active_asteroids_2(void){
    for(int i = 0; i < max_asteroids ; i++){
        if(asteroids_2[i].active) return true;
    }
    return false;
}


bool check_asteroid_spaceship_collision_2(Vector2 spaceship_position , float  spaceship_radius , int *hit_index){
    for(int i = 0 ; i < max_asteroids ; i++){
    if (!asteroids_2[i].active) continue ;
    float asteroid_r = asteroid_radius_2(asteroids_2[i].size);
    float total_rad = asteroid_r + spaceship_radius;

    if(Vector2Distance(spaceship_position , asteroids_2[i].position) <= total_rad){
        *hit_index = i;
        return true;
    }
    }
    return false;
}



void unload_asteroid_2(void){
    for(int i = 0; i < 3; i++){
    UnloadTexture(asteroid_texture_2[i]);
}
}


#define max_enemy 4
#define enemy_shot_interval 2.0f
#define enemy_direction_change_interval 3.0f

typedef enum {
    enemy1, 
    enemy2,
} enemy_type;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    enemy_type type;
    float rotation;
    float shoot_timer;
    float direction_timer;
    bool active;
} enemy;

enemy enemies[max_enemy] = {0};
Texture2D enemy_texture[2];

void InitEnemy(void) {
    enemy_texture[0] = LoadTexture("resources/EvilEye_1.png");
    enemy_texture[1] = LoadTexture("resources/EvilEye_2.png");
} 

void draw_enemy(void){
    for(int i = 0; i < max_enemy; i++){
        if(!enemies[i].active) continue;

        Texture2D enemy_tex = enemy_texture[enemies[i].type];

        Rectangle source = { 0 , 0 , (float)enemy_tex.width , (float)enemy_tex.height};
        Rectangle destination = {enemies[i].position.x , enemies[i].position.y , (float)enemy_tex.width , (float)enemy_tex.height};
        Vector2 origin = {enemy_tex.width / 2.0f , enemy_tex.height / 2.0f};

        DrawTexturePro(enemy_tex , source , destination , origin , enemies[i].rotation , WHITE);
    }

}


#define max_enemy_bullet 32
#define enemy_bullet_speed 400.0f
#define enemy_bullet_lifetime 4.0f

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float life;
    bool active;
} enemy_bullet;

enemy_bullet enemy_bullets[max_enemy_bullet] = {0};
Texture2D enemy_bullet_texture;

void InitEnemyBullet(void){
    enemy_bullet_texture = LoadTexture("resources/laserRed03.png");
}

void draw_enemy_bullet(void){
    for(int i = 0; i < max_enemy_bullet; i++){
        if(!enemy_bullets[i].active) continue;

        Rectangle source = { 0 , 0 , (float)enemy_bullet_texture.width , (float)enemy_bullet_texture.height};
        Rectangle destination = {enemy_bullets[i].position.x , enemy_bullets[i].position.y , (float)enemy_bullet_texture.width , (float)enemy_bullet_texture.height};
        Vector2 origin = {enemy_bullet_texture.width / 2.0f , enemy_bullet_texture.height / 2.0f};

        DrawTexturePro(enemy_bullet_texture, source , destination , origin , enemy_bullets[i].rotation , WHITE);
    }
}

void spawn_enemy_bullet(Vector2 enemy_position , Vector2 direction){
    for(int i = 0; i < max_enemy_bullet ; i++){
        if(enemy_bullets[i].active) continue;

        enemy_bullets[i].position = enemy_position;
        enemy_bullets[i].velocity = Vector2Scale(direction , enemy_bullet_speed);
        enemy_bullets[i].rotation = atan2f(direction.x , -direction.y)*DEG2RAD ;
        enemy_bullets[i].life = enemy_bullet_lifetime;
        enemy_bullets[i].active = true;
        return;
    }
}


void spawn_enemy(enemy_type type){
    for(int i = 0; i < max_enemy; i++){
        if(enemies[i].active) continue;

        Vector2 position;
        int edge = GetRandomValue(0,3);
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
float angle = (float)GetRandomValue(0,360)* DEG2RAD;
float speed;
if(type == enemy1 ){
speed = 120.0;
}
else{
    speed = 140.0f;
}
enemies[i].position = position;
enemies[i].velocity = (Vector2){ cosf(angle)*speed, sinf(angle)*speed };
enemies[i].rotation = atan2f(enemies[i].velocity.x , -enemies[i].velocity.y) * DEG2RAD;
enemies[i].type = type;
enemies[i].shoot_timer = enemy_shot_interval ;
enemies[i].direction_timer = enemy_direction_change_interval;
enemies[i].active = true;
return;
}
}

void enemy_shot(Vector2 enemy_position , enemy_type type , Vector2 spaceship_position ){
    Vector2 direction;

    if(type == enemy1){
        float angle = (float)GetRandomValue(0,360)*DEG2RAD;
        direction = (Vector2){cosf(angle) , sinf(angle)};
    }
    else{
        direction = Vector2Normalize(Vector2Subtract(spaceship_position, enemy_position));
    }

    spawn_enemy_bullet(enemy_position , direction);
}

void enemy_bullet_update(float dt){
    for(int i = 0; i < max_enemy_bullet ; i++){
        if(!enemy_bullets[i].active) continue;

        enemy_bullets[i].position = Vector2Add(enemy_bullets[i].position, Vector2Scale(enemy_bullets[i].velocity , dt));
        enemy_bullets[i].life = enemy_bullets[i].life - dt;

        bool off_screen = enemy_bullets[i].position.x < 0 || enemy_bullets[i].position.y < 0 || enemy_bullets[i].position.x > screenwidth || enemy_bullets[i].position.x > screenlength;

        if(enemy_bullets[i].life <= 0.0f || off_screen){
            enemy_bullets[i].active = false;
        }
    }
}
void update_enemy(float dt , Vector2 spaceship_position){
    for(int i = 0; i < max_enemy ; i++){
        if(!enemies[i].active) continue;

        enemies[i].position = Vector2Add(enemies[i].position , Vector2Scale(enemies[i].velocity , dt));

        enemies[i].direction_timer -= dt;
        if(enemies[i].direction_timer <= 0.0f){
            float angle = (float)GetRandomValue(0 , 360)*DEG2RAD;
            float speed = Vector2Length(enemies[i].velocity);
            enemies[i].velocity = (Vector2){ cosf(angle)*speed, sinf(angle)*speed };
            enemies[i].direction_timer = enemy_direction_change_interval;
        }

        enemies[i].shoot_timer -= dt;
        if(enemies[i].shoot_timer <= 0.0f){
            enemy_shot(enemies[i].position , enemies[i].type, spaceship_position);
            enemies[i].shoot_timer = enemy_shot_interval;
        }

        Texture2D tex = enemy_texture[enemies[i].type];

        if(enemies[i].position.x > screenwidth + tex.width/2.0f) {
            enemies[i].position.x = 0;
        }
        else if(enemies[i].position.x <= -tex.width/2.0f) {
            enemies[i].position.x = screenwidth;
        }
        if(enemies[i].position.y > screenlength + tex.height/2.0f) {
            enemies[i].position.y = 0;
        }
        else if(enemies[i].position.y <= - tex.height/2.0f) {
            enemies[i].position.y = screenlength;
        }
    }
}


bool enemy_bul_spaceship_collision(Vector2 spaceship_position , float spaceship_radius , int *hit_index){
    for(int i = 0 ; i < max_enemy_bullet ; i++){
        if(!enemy_bullets[i].active) continue;

        if(Vector2Distance(spaceship_position , enemy_bullets[i].position) < spaceship_radius){
            *hit_index = i;
            return true;
        }
    }
    return false;
}

bool enemy_bul_asteroid_collision(Vector2 enemy_position , float enemy_radius , int *hit_index){
    for(int i = 0 ; i < max_asteroids ; i++){
        if(!asteroids[i].active) continue;

        float asteroid_rad = asteroid_radius(asteroids[i].size);
        float combined_radius = enemy_radius + asteroid_rad;

        if(Vector2Distance(asteroids[i].position , enemy_position) < combined_radius){
            *hit_index = i;
            return true;
        }
    }
    return false;
}

bool check_enemy_hit(Vector2 bullet_position , int *hit_index){
    for(int i = 0; i < max_enemy ; i++){
        if(!enemies[i].active) continue;
        Texture2D tex = enemy_texture[enemies[i].type];
        float enemy_rad = tex.width / 2.0f;

        if(Vector2Distance(bullet_position , enemies[i].position) < enemy_rad ){
            *hit_index = i;
            return true;
        }
    }
    return false;
}

bool check_any_active_enemy(void){
    for(int i = 0; i < max_enemy ; i++){
        if(enemies[i].active) return true;
    }
    return false;
}


float enemy_spawn_timer = 0.0f;
float enemy_spawn_interval = 0.0f;

float enemy_interval(int score){
    float minimum_time = 10.0f;
    float maximum_time = 25.0f;
    float diff = fminf(score / 4000.0f , 1.0f);
    float low = maximum_time - (maximum_time - minimum_time)*diff;
    float high = low + 5.0f;

    return (float)GetRandomValue((int)(low*10.0f) , (int)(high*10.0f)) / 10.0f;


}


void unload_enemy(void){
    for(int i = 0; i < 2; i++){
    UnloadTexture(enemy_texture[i]);}}

void unload_enemy_bullet(void){
    UnloadTexture(enemy_bullet_texture);
}


void update_bullets(float dt , Vector2 spaceship_position , float spaceship_rotation, GameState state){
  shooting_cooldown = shooting_cooldown - dt;
    if(IsKeyDown (KEY_SPACE) && shooting_cooldown <= 0.0f){
        shoot_bullets(spaceship_position , spaceship_rotation); 
        shooting_cooldown = shoot_interval;
    }

    for(int i = 0; i < max_bullet ; i++){
        if(!bullets[i].active) continue;
         int hit_index;
         bool hit = false;
        
        if(state == PLAYING && check_asteroid_hit(bullets[i].position , &hit_index)){
            PlaySound(astro_bul_col);
               if(asteroids[hit_index].size == asteroid_large)
                 score += 20;
              else if(asteroids[hit_index].size == asteroid_medium)
                 score += 50;
              else if(asteroids[hit_index].size == asteroid_small)
                 score += 100;
            break_asteroid(hit_index);
            hit = true;
            bullets[i].active = false;
        }
        else if(state == PLAYING_2 && check_asteroid_hit_1(bullets[i].position , &hit_index)){
            PlaySound(steel_bul_col);
               if(asteroids_1[hit_index].size == asteroid_large_1)
                 score += 30;
              else if(asteroids_1[hit_index].size == asteroid_medium_1)
                 score += 60;
              else if(asteroids_1[hit_index].size == asteroid_small_1)
                 score += 120;
            break_asteroid_1(hit_index);
            hit = true;
            bullets[i].active = false;
        }
        else if(state == PLAYING_3 && check_asteroid_hit_2(bullets[i].position , &hit_index)){
            PlaySound(stone_bul_col);
               if(asteroids_2[hit_index].size == asteroid_large_2)
                 score += 40;
              else if(asteroids_2[hit_index].size == asteroid_medium_2)
                 score += 80;
              else if(asteroids_2[hit_index].size == asteroid_small_2)
                 score += 150;
            break_asteroid_2(hit_index);
            hit = true;
            bullets[i].active = false;
        }



        int enemy_hit_index;
        if(check_enemy_hit(bullets[i].position , &enemy_hit_index)){
             PlaySound(enemy_dying);
             enemies[enemy_hit_index].active = false;
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



int score=0;
int LIVES = 5;
int game_over = 0;
float hit_cooldown = 0.0f;

int main(void)
{
   
    InitWindow( screenwidth, screenlength , "ASTEROIDS");
    InitBullets();
    InitAsteroid();
    InitAsteroid_1();
    InitAsteroid_2();
    InitEnemy();
    InitEnemyBullet();
    InitAudio();
    
    
    SetExitKey(KEY_ESCAPE);
    //SetTargetFPS(60);

    Texture2D spaceship1_texture = LoadTexture("resources/Spaceship_3.png");
    Texture2D background1 = LoadTexture("resources/Starfield_08.png");
    Texture2D background2 = LoadTexture("resources/Purple_Nebula_04-1024x1024.png");
    Texture2D background3 = LoadTexture("resources/Green_Nebula_07-1024x1024.png");
    Texture2D story1_bg = LoadTexture("resources/STORY_1.png");
    Texture2D story2_bg = LoadTexture("resources/STORY_2.png");
    Texture2D story3_bg = LoadTexture("resources/STORY_3.png");
    Texture2D story4_bg = LoadTexture("resources/YOU_MADE_IT.png");


    Font story_font = GetFontDefault();
    float font_size = 33.0f;
    float spacing = 2;
    Color main_colour = (Color){255 , 255 , 255 , 255};
    Color shadow_colour = (Color){50, 120, 220, 255};
    Vector2 offset = {3 , 3};
    float linegap = 44;


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

        if (gameState == MENU)
{
    UpdateMusicStream(menu_music);
}
else if(gameState == PLAYING)
{
    UpdateMusicStream(bg_music);

    if(check_any_active_enemy()){
        if(!IsSoundPlaying(enemy_coming_hehe)){
            PlaySound(enemy_coming_hehe);
        }
    }
    else {
        if(IsSoundPlaying(enemy_coming_hehe)){
            StopSound(enemy_coming_hehe);
        }    
    }
}
else if(gameState == PLAYING_2)
{
    UpdateMusicStream(bg_music2);

    if(check_any_active_enemy()){
        if(!IsSoundPlaying(enemy_coming_hehe)){
            PlaySound(enemy_coming_hehe);
        }
    }
    else {
        if(IsSoundPlaying(enemy_coming_hehe)){
            StopSound(enemy_coming_hehe);
        }    
    }
}
else if(gameState == PLAYING_3)
{
    UpdateMusicStream(bg_music3);

    if(check_any_active_enemy()){
        if(!IsSoundPlaying(enemy_coming_hehe)){
            PlaySound(enemy_coming_hehe);
        }
    }
    else {
        if(IsSoundPlaying(enemy_coming_hehe)){
            StopSound(enemy_coming_hehe);
        }    
    }
}
else if(gameState == STORY_1 || gameState == STORY_2 || gameState == STORY_3 || gameState == STORY_4){
    UpdateMusicStream(story_line);
}

        float dt = GetFrameTime();
        if(gameState != MENU && gameState != GAME_OVER && gameState != STORY_1 && gameState != STORY_2 &&  gameState != STORY_3 && gameState != STORY_4 ){
        level_timer += dt;}
        else if(gameState == STORY_1 || gameState == STORY_2 ||  gameState == STORY_3 || gameState == STORY_4){
        level_timer_for_story += dt;   
        }

        switch(gameState){
            case MENU :
            {
                if(IsKeyPressed(KEY_ENTER)){
                    gameState = STORY_1;
                    StopMusicStream(menu_music);
                    PlayMusicStream(story_line);
                }
            }
            break;

            case STORY_1 :
            {
                if(level_timer_for_story >= story_timer){
                    gameState = PLAYING;
                    StopMusicStream(story_line);
                    PlayMusicStream(bg_music);
                    level_timer_for_story = 0.0f;

                for(int i = 0; i < max_bullet; i++) bullets[i].active = false;
                for(int i = 0; i < max_enemy; i++) enemies[i].active = false;
                for(int i = 0; i < max_enemy_bullet; i++) enemy_bullets[i].active = false;
                for(int i = 0; i < 4; i++) spawn_asteroid(asteroid_large);
                }
            }
            break;
            
            case PLAYING :
            {
                if(level_timer >= level_1_timer){
                    gameState = STORY_2;
                    level_timer = 0.0f;
                    StopMusicStream(bg_music);
                    PlayMusicStream(story_line);
                }
            }
            break;

            case STORY_2 :
            {
                if(level_timer_for_story >= story_timer){
                    gameState = PLAYING_2;
                    StopMusicStream(story_line);
                    PlayMusicStream(bg_music2);
                    level_timer_for_story = 0.0f;

                for(int i = 0; i < max_bullet; i++) bullets[i].active = false;
                for(int i = 0; i < max_enemy; i++) enemies[i].active = false;
                for(int i = 0; i < max_enemy_bullet; i++) enemy_bullets[i].active = false;
                for(int i = 0; i < 4; i++) spawn_asteroid_1(asteroid_large_1);
                }
            }
            break;
            
            case PLAYING_2 :
            {
                 if(level_timer >= level_2_timer){
                    gameState = STORY_3;
                    level_timer = 0.0f;
                    StopMusicStream(bg_music2);
                    PlayMusicStream(story_line);
                }

            }
            break;

            case STORY_3 :
            {
                if(level_timer_for_story >= story_timer){
                    gameState = PLAYING_3;
                     StopMusicStream(story_line);
                     PlayMusicStream(bg_music3);
                    level_timer_for_story = 0.0f;

                for(int i = 0; i < max_bullet; i++) bullets[i].active = false;
                for(int i = 0; i < max_enemy; i++) enemies[i].active = false;
                for(int i = 0; i < max_enemy_bullet; i++) enemy_bullets[i].active = false;
                for(int i = 0; i < 4; i++) spawn_asteroid_2(asteroid_large_2);
                }
            }
            break;

            case PLAYING_3 :
            {
                 if(level_timer >= level_3_timer){
                    gameState = STORY_4;
                    level_timer = 0.0f;
                    StopMusicStream(bg_music3);
                    PlayMusicStream(story_line);
                }

            }
            break;

             case STORY_4 :
            {
                if(level_timer_for_story >= story_timer){
                    gameState = GAME_OVER;
                     StopMusicStream(story_line);
                     PlayMusicStream(menu_music);
                    level_timer_for_story = 0.0f;

                }
            }
            break;

            case GAME_OVER :
            {
                if(level_timer_for_story >= story_timer){
                gameState = MENU;
                level_timer = 0.0f;
                level_timer_for_story = 0.0f;
            }

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
    
        if (gameState == PLAYING || gameState == PLAYING_2 || gameState == PLAYING_3){
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
        update_bullets(dt , spaceship_position , spaceship_rotation , gameState);
        update_enemy(dt , spaceship_position);
        enemy_bullet_update(dt); }
        float spaceship_radius = spaceship1_texture.width/2.0f;
        int spaceship_hit_index;

       if(gameState == PLAYING){
        update_asteroid(dt);
        if(LIVES > 0 && hit_cooldown <= 0.0f && check_asteroid_spaceship_collision(spaceship_position, spaceship_radius , &spaceship_hit_index)){
            PlaySound(astro_ship_col);
            LIVES--;
            hit_cooldown = 2.0f;
            spaceship_position = (Vector2) {screenwidth/2.0f , screenlength/2.0f};
            spaceship_velocity = (Vector2){0.00f , 0.00f};
        }
        
        while(!check_any_active_asteroids()){
            wave++;
            int count = wave + 4;
            for(int o = 0; o < count ; o++){
            spawn_asteroid(asteroid_large);
          }
        }
    }


        else if(gameState == PLAYING_2){
        update_asteroid_1(dt);
        if(LIVES > 0 && hit_cooldown <= 0.0f && check_asteroid_spaceship_collision_1(spaceship_position, spaceship_radius , &spaceship_hit_index)){
            PlaySound(astro_ship_col);
            LIVES--;
            hit_cooldown = 2.0f;
            spaceship_position = (Vector2) {screenwidth/2.0f , screenlength/2.0f};
            spaceship_velocity = (Vector2){0.00f , 0.00f};
        }
        
        
        while(!check_any_active_asteroids_1()){
            wave_2++;
        int count = wave_2 + 4;
            for(int o = 0; o < count ; o++){
            spawn_asteroid_1(asteroid_large);
          }
        }
    }


        else if(gameState == PLAYING_3){
        update_asteroid_2(dt);
        if(LIVES > 0 && hit_cooldown <= 0.0f && check_asteroid_spaceship_collision_2(spaceship_position, spaceship_radius , &spaceship_hit_index)){
            PlaySound(astro_ship_col);
            LIVES--;
            hit_cooldown = 2.0f;
            spaceship_position = (Vector2) {screenwidth/2.0f , screenlength/2.0f};
            spaceship_velocity = (Vector2){0.00f , 0.00f};

        }
    
        while(!check_any_active_asteroids_2()){
            wave_3++;
        int count = wave_3 + 4;
            for(int o = 0; o < count ; o++){
            spawn_asteroid_2(asteroid_large_2);
          }
        }
    }

        int  enemy_bullet_hit_index;
        if(LIVES > 0 && hit_cooldown <= 0.0f && enemy_bul_spaceship_collision(spaceship_position , spaceship_radius , &enemy_bullet_hit_index)){
            PlaySound(astro_ship_col);
            LIVES--;
            hit_cooldown = 2.0f;
            enemy_bullets[enemy_bullet_hit_index].active = false;
           spaceship_position = (Vector2) {screenwidth/2.0f , screenlength/2.0f};
            spaceship_velocity = (Vector2){0.00f , 0.00f};
        }


        if(enemy_spawn_interval <= 0.0f){
            enemy_spawn_interval = enemy_interval(score);
        }

    if(!check_any_active_enemy()){
            enemy_spawn_timer = enemy_spawn_timer + dt;
            if(enemy_spawn_timer >= enemy_spawn_interval){
                enemy_type type = (GetRandomValue(0,1) == 0) ? enemy1 : enemy2;
                spawn_enemy(type);
                enemy_spawn_timer = 0.0f;
                enemy_spawn_interval = 0.0f;
            }}
    else{
        enemy_spawn_timer = 0.0f;
    }        
      

        

        if(LIVES == 0){
            game_over = 1;
            gameState = GAME_OVER;
        }


    
    }

    if (gameState == MENU && IsKeyPressed(KEY_ENTER))
{
    StopMusicStream(menu_music);
    PlayMusicStream(bg_music);
    gameState = PLAYING;
    for (int i = 0; i < 4; i++)
    {
        spawn_asteroid(asteroid_large);
    }

}

if (gameState == GAME_OVER )
{
    LIVES = 5;
    score = 0;
    game_over = 0;

    spaceship_position = (Vector2){screenwidth/2.0f, screenlength/2.0f};
    spaceship_velocity = (Vector2){0.0f, 0.0f};
    spaceship_direction = (Vector2){cosf(DEG2RAD * (spaceship_rotation - 90)) , sinf(DEG2RAD * (spaceship_rotation - 90))};

    
    for (int i = 0; i < max_asteroids; i++)
{
    asteroids[i].active = false;
}

  for(int i = 0 ; i < max_bullet; i++){
    bullets[i].active = false;
}

  for(int i = 0; i < max_enemy; i++){
    enemies[i].active = false;
}
if(IsKeyPressed(KEY_R)){
gameState = PLAYING;}

for (int i = 0; i < 4; i++)
{
    spawn_asteroid(asteroid_large);
}

level_timer == 0.0f;
  
}







BeginDrawing();
    
        if (gameState == MENU)
{   DrawRectangle(0 , 0 , screenwidth, screenlength, (Color){5, 8, 25, 255});
   

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
    500,
    700,
    300,
    70
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

if(gameState == STORY_1){
    Rectangle source1 = {0 , 0 , (float)story1_bg.width , (float)story1_bg.height};
    Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
    Vector2 origin1 = {0,0};
    DrawTexturePro(story1_bg, source1 , dest1 , origin1 , 0.0f , WHITE);
    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100 + offset.x , 800} , font_size ,spacing , shadow_colour );
    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100  , 800} , font_size ,spacing , main_colour );

}       
if (gameState == PLAYING || gameState == PLAYING_2 || gameState == PLAYING_3)
{
    if(gameState == PLAYING){
        Rectangle source1 = {0 , 0 , (float)background1.width , (float)background1.height};
        Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
        Vector2 origin1 = {0,0};
        DrawTexturePro(background1 , source1 , dest1 , origin1 , 0.0f , WHITE);

         draw_asteroid();

    }
    else if(gameState == PLAYING_2){
        Rectangle source1 = {0 , 0 , (float)background2.width , (float)background2.height};
        Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
        Vector2 origin1 = {0,0};
        DrawTexturePro(background2 , source1 , dest1 , origin1 , 0.0f , WHITE);

        draw_asteroid_1();

    }
    else if(gameState == PLAYING_3){
        Rectangle source1 = {0 , 0 , (float)background3.width , (float)background3.height};
        Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
        Vector2 origin1 = {0,0};
        DrawTexturePro(background3 , source1 , dest1 , origin1 , 0.0f , WHITE);

        draw_asteroid_2();

    }
        
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
     DrawText(TextFormat("TIME: %.1f" , level_timer) , 20 , 90 , 30 ,WHITE);

        draw_bullets();
        draw_enemy();
        draw_enemy_bullet();
        
 }

    if(gameState == STORY_2){
    Rectangle source1 = {0 , 0 , (float)story2_bg.width , (float)story2_bg.height};
    Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
    Vector2 origin1 = {0,0};
    DrawTexturePro(story2_bg, source1 , dest1 , origin1 , 0.0f , WHITE);

    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100 + offset.x , 800} , font_size ,spacing , shadow_colour );
    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100  , 800} , font_size ,spacing , main_colour );
    }

    if(gameState == STORY_3){
    Rectangle source1 = {0 , 0 , (float)story3_bg.width , (float)story3_bg.height};
    Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
    Vector2 origin1 = {0,0};
    DrawTexturePro(story3_bg, source1 , dest1 , origin1 , 0.0f , WHITE);

    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100 + offset.x , 800} , font_size ,spacing , shadow_colour );
    DrawTextEx(story_font ,"PRESS ENTER TO SKIP",(Vector2){1100  , 800} , font_size ,spacing , main_colour );
    }


    if(gameState == STORY_4){
    Rectangle source1 = {0 , 0 , (float)story4_bg.width , (float)story4_bg.height};
    Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
    Vector2 origin1 = {0,0};
    DrawTexturePro(story4_bg, source1 , dest1 , origin1 , 0.0f , WHITE);

    }

    if (gameState == GAME_OVER)
{
    DrawText("GAME OVER", screenwidth/2.0f - 250, screenlength/2.0f - 50, 80, WHITE);
    DrawText("PRESS R TO RESTART", screenwidth/2.0f - 170, screenlength/2.0f + 50, 30, WHITE);
}
        DrawFPS(0 , 0);

        EndDrawing();
    }


    unload_asteroid();
    unload_bullets();
    unload_audio();
    unload_enemy();
    unload_enemy_bullet();
    UnloadTexture(spaceship1_texture);
    UnloadTexture(background1);
    UnloadTexture(background2);
    UnloadTexture(background3);
    UnloadTexture(story1_bg);
    UnloadTexture(story2_bg);
    UnloadTexture(story3_bg);
    UnloadTexture(story4_bg);
    CloseWindow(); 

    return 0;
}
