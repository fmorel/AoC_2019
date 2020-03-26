
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intcode.h"

static int64_t get_param(const Context *ctx, int par_mode, int64_t op)
{
    switch(par_mode) {
        case 0:
            /* Position mode */
            return ctx->program[op];
        case 1:
            /* Immediate mode */
            return op;
        case 2:
            /* Relative mode */
            return ctx->program[op + ctx->relative_base];
    }
}

static void set_param(Context *ctx, int par_mode, int64_t op, int64_t value)
{
     switch(par_mode) {
        case 0:
            /* Position mode */
            ctx->program[op] = value;
            break;
        case 1:
            /* Immediate mode */
            printf("Error, setting value in immediate mode\n");
            exit(-1);
        case 2:
            /* Relative mode */
            ctx->program[op + ctx->relative_base] = value;
            break;
    }
}

int run_inst(Context *ctx, int position)
{
    int64_t op1, op2, op3, inst, opcode;
    int par_mode[3], i;
    int64_t *prog = ctx->program;

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
            /* Add */
            set_param(ctx, par_mode[2], op3,
                get_param(ctx, par_mode[0], op1) + 
                get_param(ctx, par_mode[1], op2)
            );
            return position + 4;
        case 2:
            /* Mult */
            set_param(ctx, par_mode[2], op3,
                get_param(ctx, par_mode[0], op1) * 
                get_param(ctx, par_mode[1], op2)
            );
            return position + 4;
        case 3:
            /* Store input */
            if (ctx->pause_on_input) {
                ctx->pause_on_input = 0;
                return position;
            }
            set_param(ctx, par_mode[0], op1,
                ctx->input[ctx->input_idx++]);
            return position + 2;
        case 4:
            /* Output */
            ctx->output[ctx->output_idx++] = get_param(ctx, par_mode[0], op1);
            ctx->output_triggered = 1;
            return position + 2;
        case 5:
            /* Jump if true */
            if (get_param(ctx, par_mode[0], op1))
                return get_param(ctx, par_mode[1], op2);
            return position + 3;
        case 6:
            /* Jump if false */
            if (!get_param(ctx, par_mode[0], op1))
                return get_param(ctx, par_mode[1], op2);
            return position + 3;
        case 7:
            /* Less than */
            set_param(ctx, par_mode[2], op3,
                (get_param(ctx, par_mode[0], op1) < get_param(ctx, par_mode[1], op2))
            );
            return position + 4;
        case 8:
            /* Equal */
            set_param(ctx, par_mode[2], op3,
                (get_param(ctx, par_mode[0], op1) == get_param(ctx, par_mode[1], op2))
            );
            return position + 4; 
        case 9:
            /* Relative base offset */
            ctx->relative_base += get_param(ctx, par_mode[0], op1);
            return position + 2;
        default:
            printf("Opcode error\n");
            return -1;
    }
}

int run(Context *ctx)
{
    int position = 0;
    ctx->relative_base = 0;

    while(position >= 0)
       position = run_inst(ctx, position);
    return ctx->program[0];
}

int resume_till_event(Context *ctx, int *p_event, int cur_pos)
{
    int input_bit = (*p_event & INTCODE_EVENT_INPUT);
    int output_bit = (*p_event & INTCODE_EVENT_OUTPUT);
    
    if (input_bit)
        ctx->pause_on_input = 1;
    if (output_bit)
        ctx->output_triggered = 0;
        
    while (cur_pos >=0) {
        cur_pos = run_inst(ctx, cur_pos);
        if (input_bit && !ctx->pause_on_input) {
            *p_event = input_bit;
            break;
        }
        if (output_bit && ctx->output_triggered) {
            *p_event = output_bit;
            break;
        }
    }

    return cur_pos;
}

int resume_till_output(Context *ctx, int cur_pos)
{
    int event = INTCODE_EVENT_OUTPUT;
    return resume_till_event(ctx, &event, cur_pos);
}

int parse_program(const char *filename, int64_t *program)
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
        program[i++] = strtoll(token, NULL, 10);
    }
    free(token);
    printf("Program size %d\n", i);
    printf("Program start is %ld %ld %ld %ld ...\n", program[0], program[1], program[2], program[3]);
    printf("Program end is %ld %ld\n", program[i-2], program[i-1]);
    return i;
}
