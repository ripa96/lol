#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

#define GAME_WIDTH 800
#define GAME_HEIGHT 600

#define BOX_WIDTH 25
#define BOX_HEIGHT 25
#define OFF_THE_WALL 30

#define PLAYER_LEN 6

#define INCREASE_SPEED_BY 20
#define PLAYER_SPEED 120
float BALL_SPEED = 100;

Uint64 last_tick = 0;
Uint64 current_tick = 0;
float delta_time;

static SDL_Window *window;
static SDL_Renderer *renderer;

int Score_Player1 = 0;
int Score_Player2 = 0;

typedef struct {
    float x, y, w, h;
    float vx, vy;
} Box;

Box player1[PLAYER_LEN - 1];
Box player2[PLAYER_LEN - 1];

Box ball = {
    .x = (GAME_WIDTH / 2) - (BOX_WIDTH / 2),
    .y = (GAME_HEIGHT / 2) - (BOX_HEIGHT / 2),
    .w = BOX_WIDTH,
    .h = BOX_HEIGHT,
    .vx = 1,
    .vy = 0
};



SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){

    if (!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow(
        "Pong",
        GAME_WIDTH,
        GAME_HEIGHT,
        0
    );

    if (!window){
        SDL_Log("Error creating window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer){
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    for (int i = 0; i < PLAYER_LEN - 1; i++){
        player1[i].x = OFF_THE_WALL;
        player1[i].y = ((GAME_HEIGHT / 2) - (BOX_HEIGHT * (PLAYER_LEN / 2))) + i * BOX_HEIGHT;
        player1[i].w = BOX_WIDTH;
        player1[i].h = BOX_HEIGHT;
        player1[i].vy = 0;
    }

    for (int i = 0; i < PLAYER_LEN - 1; i++){
        player2[i].x = GAME_WIDTH - OFF_THE_WALL - BOX_WIDTH;
        player2[i].y = ((GAME_HEIGHT / 2) - (BOX_HEIGHT * (PLAYER_LEN / 2))) + i * BOX_HEIGHT;
        player2[i].w = BOX_WIDTH;
        player2[i].h = BOX_HEIGHT;
        player2[i].vy = 0;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT){
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}


/* ------------------- COLLISIONS --------------------- */

int paddle_collision_wall_1(){
    
    if (player1[0].y < 0){
        return 1;
    }
    
    if ((player1[PLAYER_LEN - 2].y + BOX_HEIGHT) > GAME_HEIGHT){
        return 2;
    }

    return 0;
}

int paddle_collision_wall_2(){
    
    if (player2[0].y < 0){
        return 1;
    }
    
    if ((player2[PLAYER_LEN - 2].y + BOX_HEIGHT) > GAME_HEIGHT){
        return 2;
    }

    return 0;
}

int paddle_collision_ball(){

    /* --- PLAYER 1 ----*/
    
    if (player1[0].x < (ball.x + BOX_WIDTH) && ball.x < (player1[0].x + BOX_WIDTH) && 
    player1[0].y < (ball.y + BOX_HEIGHT) &&
    (player1[PLAYER_LEN - 2].y + BOX_HEIGHT) > ball.y)
    {
        float MAX_HIT = (PLAYER_LEN - 1) * BOX_HEIGHT + BOX_HEIGHT;
        float paddle_hit = (MAX_HIT / 2) - (ball.y - (player1[0].y - BOX_HEIGHT));

        if(paddle_hit == 0){
            paddle_hit = 0.001;
        }

        if (paddle_hit <= MAX_HIT / 2){
            ball.vy = -(1 / ((MAX_HIT / 2) / paddle_hit));
        }

        ball.x = OFF_THE_WALL + BOX_WIDTH;
        ball.vx = -ball.vx;
        BALL_SPEED += INCREASE_SPEED_BY;
    }


    /* ----- PLAYER 2 ------ */

    if (player2[0].x < (ball.x + BOX_WIDTH) && ball.x < (player2[0].x + BOX_WIDTH) && 
    player2[0].y < (ball.y + BOX_HEIGHT) &&
    (player2[PLAYER_LEN - 2].y + BOX_HEIGHT) > ball.y)
    {
        float MAX_HIT = (PLAYER_LEN - 1) * BOX_HEIGHT + BOX_HEIGHT;
        float paddle_hit = (MAX_HIT / 2) - (ball.y - (player2[0].y - BOX_HEIGHT));

        if(paddle_hit == 0){
            paddle_hit = 0.01;
        }

        if (paddle_hit <= MAX_HIT / 2){
            ball.vy = -(1 / ((MAX_HIT / 2) / paddle_hit));
        }

        ball.x = GAME_WIDTH - OFF_THE_WALL - 2 * BOX_WIDTH;
        ball.vx = -ball.vx;
        BALL_SPEED += INCREASE_SPEED_BY;
    }    

    return 0;
}

void ball_collision_wall(){

    if (ball.x < 0){
        ++Score_Player2;

        ball.x = GAME_WIDTH / 2;
        ball.y = GAME_HEIGHT / 2;
        BALL_SPEED = 100;
        ball.vy = 0;
    }
    
    if (ball.x > GAME_WIDTH - ball.w){
        ++Score_Player1;

        ball.x = GAME_WIDTH / 2;
        ball.y = GAME_HEIGHT / 2;
        BALL_SPEED = 100;
        ball.vy = 0;
    }


    if ((ball.y) < 0){
        ball.y = -ball.y;
        ball.vy = -ball.vy;
    } 
        
    if ((ball.y + ball.h) > GAME_HEIGHT){
        ball.y = GAME_HEIGHT - (GAME_HEIGHT - ball.y);
        ball.vy = -ball.vy;
    } 
}



/* --------------------- UPDATE PADDLES ----------------------- */

static void update_paddles(float delta_time){
    
    const _Bool *keyboard_state = SDL_GetKeyboardState(NULL);

    /* ------- PLAYER 1 ------- */

    if (keyboard_state[SDL_SCANCODE_W]){
        if (paddle_collision_wall_1() == 0 || paddle_collision_wall_1() == 2){
            for (int i = 0; i < PLAYER_LEN - 1; i++){
                player1[i].y -= PLAYER_SPEED * delta_time;
            }
        }
    }
   
    if (keyboard_state[SDL_SCANCODE_S]){
        if (paddle_collision_wall_1() == 0 || paddle_collision_wall_1() == 1){
            for (int i = 0; i < PLAYER_LEN - 1; i++){
                player1[i].y += PLAYER_SPEED * delta_time;
            }
        }
    }
   
    /* ---- PLAYER 2 ----*/

   if (keyboard_state[SDL_SCANCODE_UP]){
        if (paddle_collision_wall_2() == 0 || paddle_collision_wall_2() == 2){
            for (int i = 0; i < PLAYER_LEN - 1; i++){
                player2[i].y -= PLAYER_SPEED * delta_time;
            }
        }
    }

    if (keyboard_state[SDL_SCANCODE_DOWN]){
        if (paddle_collision_wall_2() == 0 || paddle_collision_wall_2() == 1){
            for (int i = 0; i < PLAYER_LEN - 1; i++){
                player2[i].y += PLAYER_SPEED * delta_time;
            }
        }
    }

}

/* -------------------- UPDATE BALL -------------------------- */

void update_ball(float delta_time){
    ball_collision_wall(); //must happen before paddle collision
    paddle_collision_ball();


    ball.x = ball.x + ball.vx * delta_time * BALL_SPEED;
    ball.y = ball.y + ball.vy * delta_time * BALL_SPEED;
}

/* ------ UPDATE ALL ------ */


void update_all(){
    last_tick = current_tick;
    current_tick = SDL_GetTicks();
    delta_time = (current_tick - last_tick) / 1000.0f;

    update_ball(delta_time);
    update_paddles(delta_time);

}


/* -------------------- RENDER ALL ------------------ */

void render_ball(){

    SDL_FRect ball_rect = {
        .x = ball.x,
        .y = ball.y,
        .w = ball.w,
        .h = ball.h
    };
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); 
    SDL_RenderFillRect(renderer, &ball_rect);

}

void render_players(){

    for (int i = 0; i < PLAYER_LEN - 1; i++){
        SDL_FRect player1_rect = {
            .x = player1[i].x,
            .y = player1[i].y,
            .w = player1[i].w,
            .h = player1[i].h,
        };
        SDL_RenderFillRect(renderer, &player1_rect);
    }

    for (int i = 0; i < PLAYER_LEN - 1; i++){
        SDL_FRect player2_rect = {
            .x = player2[i].x,
            .y = player2[i].y,
            .w = player2[i].w,
            .h = player2[i].h,
        };
        SDL_RenderFillRect(renderer, &player2_rect);
    }
}

void render_score(){
    SDL_SetRenderScale(renderer, 2.0f, 2.0f);
    SDL_RenderDebugTextFormat(renderer, GAME_WIDTH / 8, GAME_HEIGHT / 10, "Score: %d", Score_Player1);
    SDL_RenderDebugTextFormat(renderer, GAME_WIDTH / 3.5, GAME_HEIGHT / 10, "Score: %d", Score_Player2);

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

void render_all(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
    SDL_RenderClear(renderer);
    
    render_ball();
    render_players();
    render_score();

    SDL_RenderPresent(renderer);
}



/* --------------- MAIN FUNKTION --------------------------- */

SDL_AppResult SDL_AppIterate(void *appstate){
    update_all();
    render_all();

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result){

}