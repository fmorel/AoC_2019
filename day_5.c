#include <stdio.h>
#include <stdlib.h>

#include "intcode.h"

#define PROGRAM_SIZE 768

int main(int argc, char **argv)
{
    int64_t program[PROGRAM_SIZE];
    int64_t input[1];
    int64_t output[64];
    int size, i;
    Context ctx;

    input[0] = atoi(argv[2]);

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
