
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
    SDL_Texture *buffer;
} SDLContext;

#define SDL_ERROR(str) \
    printf(#str "error: %d\n", SDL_GetError()); \
    SDL_Quit(); \
    exit(-1)

#define TILE_SIZE 16    /* Tile size in pixel */
#define WIDTH 44        /* Game width and height, in tiles */
#define HEIGHT 22
#define TICK_MS 300

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
    /* Framebuffer */
    c->buffer = SDL_CreateTexture(c->ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!c->buffer) {
        SDL_DestroyRenderer(c->ren);
        SDL_DestroyWindow(c->win);
        SDL_ERROR(Buffer);
    }
    SDL_SetRenderTarget(c->ren, c->buffer);
    
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
    SDL_DestroyTexture(c->buffer);
    SDL_DestroyRenderer(c->ren);
    SDL_DestroyWindow(c->win);
    SDL_Quit();
}

void sdl_draw_tile(SDLContext *c, int64_t *data)
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

void sdl_swap_buffer(SDLContext *c)
{
    SDL_SetRenderTarget(c->ren, NULL);
    SDL_RenderClear(c->ren);
    SDL_RenderCopy(c->ren, c->buffer, NULL, NULL);
    SDL_RenderPresent(c->ren);
    SDL_SetRenderTarget(c->ren, c->buffer);
}

int run_step(Context *ctx, int pc)
{
    ctx->output_idx = 0;
    pc = resume_till_output(ctx, pc);
    if (pc < 0)
        return -1;
    pc = resume_till_output(ctx, pc);
    if (pc < 0)
        return -1;
    pc = resume_till_output(ctx, pc);
    return pc;
}

int run_program(Context *ctx, SDLContext *c)
{
    int pc = 0;
    int n_loop = 0;
    int64_t input = 0;
    SDL_Event event;

    /* Main game loop */
    while (pc >= 0) {
        /* Fetch input */
        ctx->input[0] = input;
        ctx->input_idx = 0;
        input = 0;
        
        /* Draw loop : run until another event is requested */
        while (pc >= 0) {
            pc = run_step(ctx, pc);
            /* Don't draw the last tile, it will erase the ball ... */
            if (ctx->input_idx != 0)
                break;
            sdl_draw_tile(c, ctx->output);
        }
        /* Display game */
        sdl_swap_buffer(c);
        /* Draw the last tile */
        sdl_draw_tile(c, ctx->output);

        /* Retrieve all the events of the last tick : last one wins */
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_KEYDOWN) {
                SDL_KeyboardEvent *key = &event.key;
                if (key->keysym.sym == SDLK_RIGHT)
                    input = 1;
                else if (key->keysym.sym == SDLK_LEFT)
                    input = -1;
            }
        }
        /* Delay before next tick */
        SDL_Delay(TICK_MS);
        n_loop++;
    }
}


int main(int argc, char **argv)
{
    int64_t *program;
    Context ctx;
    SDLContext c;
    int size; 
    
    program = malloc(PROGRAM_SIZE * sizeof(int64_t));
    size = parse_program(argv[1], program);
    if (size > PROGRAM_SIZE) {
        printf("Program too big %d\n", size);
        return -1;
    }

    program[0] = 2;

    ctx.program = program;
    ctx.relative_base = 0;
    ctx.input = malloc(2 * sizeof(int64_t));
    ctx.output = malloc(3 * sizeof(int64_t));
    ctx.input_idx = 0;

    sdl_init(&c);
    
    run_program(&ctx, &c);
    
    sdl_end(&c);
    free(program); 
}

