#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SIZE 256
#define LINE_SIZE 512


int run(int32_t *program, int position)
{
    int op1, op2, res;

    if (program[position] == 99) {
        printf("End of program .. program[0] is %d\n", program[0]);
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

void parse_program(const char *filename, int32_t *program)
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
}



int main(int argc, char **argv) {
    int position = 0;
    int32_t program[PROGRAM_SIZE];

    parse_program(argv[1], program);
    
    while(position >= 0)
       position = run(program, position);

    return 0;
}


