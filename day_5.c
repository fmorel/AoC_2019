#include <stdio.h>

#include "intcode.h"

#define PROGRAM_SIZE 768

int main(int argc, char **argv)
{
    int32_t program[PROGRAM_SIZE];
    int input[1] = {1};
    int output[64];
    int size, i;
    Context ctx;

    ctx.input = input;
    ctx.input_idx = 0;
    ctx.output = output;
    ctx.output_idx = 0;
    size = parse_program(argv[1], program);
    ctx.program = program;

    run(&ctx);

    for (i = 0; i < ctx.output_idx; i++) {
        printf("Out[%d] : %d\n", i, ctx.output[i]);
    }

    return 0;
}
