#include "raylib.h"
#include "raymath.h"
#include<stdio.h>
#include<stdlib.h>

#define screenwidth  1500
#define screenlength  900

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
        case asteroid_large : return 55.0f;
        case asteroid_medium : return 45.0f;
        case asteroid_small : return 30.0f;
    }
    return 55.0f;
}

void InitAsteroid(void){
    asteroid_texture[0] = LoadTexture("resources/Asteroid_1.png");
    asteroid_texture[1] = LoadTexture("resources/Asteroid_3.png");
    asteroid_texture[2] = LoadTexture("resources/Asteroid_5.png");
}

void spawn_asteroid(asteroid_size size) {
    for(int i = 0; i <= max_asteroids ; i++){
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

  void unload_asteroid(void){
    for(int i = 0; i < 3; i++){
    UnloadTexture(asteroid_texture[i]);
}
}



int main(void)
{
   
    InitWindow( screenwidth, screenlength , "Spaceship");
    InitBullets();
    InitAsteroid();
    for(int o = 0; o < 4 ; o++){
        spawn_asteroid(asteroid_large);
    }
    
    SetExitKey(KEY_ESCAPE);
    //SetTargetFPS(60);
    Texture2D spaceship1_texture = LoadTexture("resources/Spaceship_3.png");
    Texture2D background1 = LoadTexture("resources/Starfield_08.png");
    Vector2 spaceship_position = (Vector2){screenwidth/2.0f , screenlength/2.0f};
    Vector2 spaceship_velocity = (Vector2){0.0f , 0.0f};
   
    float spaceship_rotation = 0.00f;
    float spaceship_rotation_speed = 180.00f;
    float spaceship_base_speed = 0.00f;
    float thrust = 600;
    float friction_per_second = 0.6f;

    

    while (!WindowShouldClose())
    {

        float dt = GetFrameTime();
        Vector2 spaceship_direction = (Vector2){cosf(DEG2RAD * (spaceship_rotation - 90)) , sinf(DEG2RAD * (spaceship_rotation - 90))};

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
    


        BeginDrawing();
        Rectangle source1 = {0 , 0 , (float)background1.width , (float)background1.height};
        Rectangle dest1 = {0 , 0 , (float)screenwidth , (float)screenlength};
        Vector2 origin1 = {0,0};
        DrawTexturePro(background1 , source1 , dest1 , origin1 , 0.0f , WHITE);
        

        
    
       for(int i = 0 ; i < max_fire_particle ; i++){
        if(fire_particles[i].active){
            float time = fire_particles[i].life / fire_particles[i].max_life;
            Color fire = { 200 , (unsigned char)(60*time) , 0 , (unsigned char)(160*time)};
            float flicker = 1.0f + 0.25f * sinf(fire_particles[i].life * 50.0f + i);
            float radius = 3.0f*time*flicker;
            DrawCircleV(fire_particles[i].position , radius , fire);
        }
       }

        Rectangle source2 = {0 , 0 , (float)spaceship1_texture.width , (float)spaceship1_texture.height};
        Rectangle dest2 = {spaceship_position.x , spaceship_position.y ,(float)spaceship1_texture.width , (float)spaceship1_texture.height };
        Vector2 origin = {spaceship1_texture.width/2.0f , spaceship1_texture.height/2.0f};
        DrawTexturePro(spaceship1_texture , source2 , dest2 , origin , spaceship_rotation , WHITE );

        draw_bullets();
        draw_asteroid();
        

        DrawFPS(0 , 0);

        EndDrawing();
    }


    unload_bullets();
    CloseWindow(); 

    return 0;
}
