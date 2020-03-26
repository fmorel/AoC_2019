
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "intcode.h"

#include <SDL2/SDL.h>

#define PROGRAM_SIZE 4096

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture *tiles;
} SDLContext;

typedef struct {
    SDLContext sdl; /* Graphics */
    Context ctx;    /* Intcode Game engine */
    int x_ball;
    int x_paddle;
} Game;

#define SDL_ERROR(str) \
    printf(#str "error: %d\n", SDL_GetError()); \
    SDL_Quit(); \
    exit(-1)

#define TILE_SIZE 16    /* Tile size in pixel */
#define WIDTH 44        /* Game width and height, in tiles */
#define HEIGHT 22
#define TICK_MS 200

void sdl_init(SDLContext *c)
{
    int w, h;
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_ERROR(Init);
    }
    w = (WIDTH + 3) * TILE_SIZE;
    h = (HEIGHT + 3) * TILE_SIZE;
    /* Create window */
    c->win = SDL_CreateWindow("Elves breakout", 100, 100, w, h, SDL_WINDOW_SHOWN);
    if (!c->win){
        SDL_ERROR(Window);
    }
    /* Renderer */
    c->ren = SDL_CreateRenderer(c->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!c->ren) {
	SDL_DestroyWindow(c->win);
        SDL_ERROR(Renderer);
    }
    
    /* Load tiles texture */
    SDL_Surface *img = SDL_LoadBMP("breakout_tiles.bmp"); 
    if(!img) {
        SDL_DestroyRenderer(c->ren);
        SDL_DestroyWindow(c->win);
        SDL_ERROR(Image);
    }
    c->tiles = SDL_CreateTextureFromSurface(c->ren, img);
    SDL_FreeSurface(img);

}

void sdl_end(SDLContext *c)
{
    SDL_DestroyTexture(c->tiles);
    SDL_DestroyRenderer(c->ren);
    SDL_DestroyWindow(c->win);
    SDL_Quit();
}

void sdl_draw_tile(SDLContext *c, int64_t *data, int *x_ball, int *x_paddle)
{
    int x, y, tile;
    SDL_Rect src, dst;
    
    x = data[0];
    y = data[1];
    tile = data[2];

    if (x == -1 && y == 0) {
        printf("New score %d\n", tile);
        return;
    }
    if (tile == 4)
        *x_ball = x;
    if (tile == 3)
        *x_paddle = x;
    /* Tiles are arrenged horizontally in texture tile */
    src.x = tile * TILE_SIZE;
    src.y = 0;
    src.w = TILE_SIZE;
    src.h = TILE_SIZE;
    
    /* Leave a 1 tile border space */
    dst.x = (x + 1) * TILE_SIZE;
    dst.y = (y + 1) * TILE_SIZE;
    dst.w = TILE_SIZE;
    dst.h = TILE_SIZE;

    /* Draw texture */
    SDL_RenderCopy(c->ren, c->tiles, &src, &dst);
}


int run_tick(Game *g, int pc)
{
    int event;
    while (pc >= 0) {
        g->ctx.output_idx = 0;
        event = INTCODE_EVENT_INPUT | INTCODE_EVENT_OUTPUT;
        pc = resume_till_event(&g->ctx, &event, pc);
        if (pc < 0)
            return -1;
        if (event == INTCODE_EVENT_INPUT)
            break;
        pc = resume_till_output(&g->ctx, pc);
        pc = resume_till_output(&g->ctx, pc);

        sdl_draw_tile(&g->sdl, g->ctx.output, &g->x_ball, &g->x_paddle);
    }
    return pc;
}

void run_program(Game *g, int autoplay)
{
    int pc = 0;
    int n_loop = 0;
    int64_t input = 0;
    int x_ball, x_paddle;
    SDL_Event event;

    /* Main game loop */
    while (pc >= 0) {
        /* Draw loop : run until another event is requested */
        pc = run_tick(g, pc);
        if (pc < 0)
            break;
        
        /* Display game */
        SDL_RenderPresent(g->sdl.ren);
        
        /* Wait and fetch input */
        input = 0;
        if (!autoplay) {
            SDL_Delay(TICK_MS);
            /* Retrieve all the events : last one wins */
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_KEYDOWN) {
                    SDL_KeyboardEvent *key = &event.key;
                    if (key->keysym.sym == SDLK_RIGHT)
                        input = 1;
                    else if (key->keysym.sym == SDLK_LEFT)
                        input = -1;
                } else if (event.type == SDL_QUIT) {
                    printf("Force quit\n");
                    return;
                }
            }
        } else {
            SDL_Delay(TICK_MS/10);

            if (g->x_paddle < g->x_ball)
                input = 1;
            else if (g->x_paddle > g->x_ball)
                input = -1;
        }

        /* Run the single input instruction */
        g->ctx.input[0] = input;
        g->ctx.input_idx = 0;
        pc = run_inst(&g->ctx, pc);

        n_loop++;
    }
}


int main(int argc, char **argv)
{
    int64_t *program;
    Game g;
    int size; 
    
    program = malloc(PROGRAM_SIZE * sizeof(int64_t));
    size = parse_program(argv[1], program);
    if (size > PROGRAM_SIZE) {
        printf("Program too big %d\n", size);
        return -1;
    }

    program[0] = 2;

    g.ctx.program = program;
    g.ctx.relative_base = 0;
    g.ctx.input = malloc(1 * sizeof(int64_t));
    g.ctx.output = malloc(3 * sizeof(int64_t));

    sdl_init(&g.sdl);
    
    run_program(&g, atoi(argv[2]));
    
    sdl_end(&g.sdl);
    free(program); 
}

