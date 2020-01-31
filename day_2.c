#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SIZE 256
#define LINE_SIZE 512


int run_inst(int32_t *program, int position)
{
    int op1, op2, res;

    if (program[position] == 99) {
        return -1;
    }

    op1 = program[position + 1];
    op2 = program[position + 2];
    res = program[position + 3];
    
    if (program[position] == 1) {
        program[res] = program[op1] + program[op2];
        return position + 4;
    }
    if (program[position] == 2) {
        program[res] = program[op1] * program[op2];
        return position + 4;
    }
    printf("Opcode error\n");
    return -1;
}

int run(int32_t *program)
{
    int position = 0;
    while(position >= 0)
       position = run_inst(program, position);
    return program[0];
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



int main(int argc, char **argv) {
    int ip = 0, size;
    int noun, verb;
    int32_t program[PROGRAM_SIZE];
    int32_t program_instance[PROGRAM_SIZE];

    size = parse_program(argv[1], program);
    for (noun = 0; noun <= 99; noun++) {
        for (verb = 0; verb <= 99; verb++) {
            memcpy(program_instance, program, 4 * size);
            program_instance[1] = noun;
            program_instance[2] = verb;
            if (run(program_instance) == 19690720)
                break;
        }
        if (verb != 100)
            break;
    }
    printf("noun %d, verb %d\n", noun, verb);
    return 0;
}


