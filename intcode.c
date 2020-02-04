
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intcode.h"

static int get_param(const int32_t *program, int par_mode, int op)
{
    /* Position mode */
    if (par_mode == 0)
        return program[op];
    /* Immediate mode */
    return op;
}

static int run_inst(Context *ctx, int position)
{
    int op1, op2, op3, inst, opcode, i;
    int par_mode[3];
    int32_t *prog = ctx->program;

    inst = prog[position];
    if (inst == 99) {
        return -1;
    }

    op1 = prog[position + 1];
    op2 = prog[position + 2];
    op3 = prog[position + 3];
    
    opcode = inst % 100;
    inst /= 100;
    for (i = 0; i < 3; i++) {
        par_mode[i] = inst % 10;
        inst /= 10;
    }
    switch(opcode) {
        case 1:
            prog[op3] = get_param(prog, par_mode[0], op1) + 
                        get_param(prog, par_mode[1], op2);
            return position + 4;
        case 2:
            prog[op3] = get_param(prog, par_mode[0], op1) * 
                        get_param(prog, par_mode[1], op2);
            return position + 4;
        case 3:
            prog[op1] = ctx->input[ctx->input_idx++];
            return position + 2;
        case 4:
            ctx->output[ctx->output_idx++] = get_param(prog, par_mode[0], op1);
            return position + 2;
        default:
            printf("Opcode error\n");
            return -1;
    }
}

int run(Context *ctx)
{
    int position = 0;
    while(position >= 0)
       position = run_inst(ctx, position);
    return ctx->program[0];
}

int parse_program(const char *filename, int32_t *program)
{
    int i = 0;
    FILE *f = fopen(filename, "r");
    char *token = NULL;
    size_t token_size = 1;
    const char sep[2] = ",";

    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }
    while (getdelim(&token, &token_size, ',', f) > 0) {
        program[i++] = atoi(token);
    }
    free(token);
    printf("Program size %d\n", i);
    printf("Program start is %d %d %d %d ...\n", program[0], program[1], program[2], program[3]);
    printf("Program end is %d %d\n", program[i-2], program[i-1]);
    return i;
}
