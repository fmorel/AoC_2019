
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "intcode.h"

#define PROGRAM_SIZE 4096


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

int main(int argc, char **argv)
{
    int64_t *program;
    Context ctx;
    int size, pc = 0; 
    int n_block = 0, n_loop = 0;
    
    program = malloc(PROGRAM_SIZE * sizeof(int64_t));
    size = parse_program(argv[1], program);
    if (size > PROGRAM_SIZE) {
        printf("Program too big %d\n", size);
        return -1;
    }

    ctx.program = program;
    ctx.relative_base = 0;
    ctx.input = malloc(1 * sizeof(int64_t));
    ctx.output = malloc(3 * sizeof(int64_t));
    ctx.input_idx = 0;

    while (pc >= 0) {
        pc = run_step(&ctx, pc);
        n_loop++;
        if (ctx.output[ctx.output_idx-1] == 2)
            n_block++;
    }
    printf("%d block tiles (%d loops)\n", n_block, n_loop);
}

