#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intcode.h"

#define PROGRAM_SIZE 256


int main(int argc, char **argv) {
    int ip = 0, size;
    int noun, verb;
    int32_t program[PROGRAM_SIZE];
    int32_t program_instance[PROGRAM_SIZE];
    Context ctx;

    ctx.input = NULL;
    ctx.output = NULL;
    size = parse_program(argv[1], program);
    for (noun = 0; noun <= 99; noun++) {
        for (verb = 0; verb <= 99; verb++) {
            memcpy(program_instance, program, 4 * size);
            program_instance[1] = noun;
            program_instance[2] = verb;
            ctx.program = program_instance;
            if (run(&ctx) == 19690720)
                break;
        }
        if (verb != 100)
            break;
    }
    printf("noun %d, verb %d\n", noun, verb);
    return 0;
}


