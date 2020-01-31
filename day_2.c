#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SIZE 256
#define LINE_SIZE 512


int run(int16_t *program, int position)
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

void parse_program(const char *filename, int16_t *program)
{
    int i = 0;
    FILE *f = fopen(filename, "r");
    char line[LINE_SIZE];
    char *token;
    const char sep[2] = ",";

    if (!f) {
        printf("COuld not open file\n");
        exit(-1);
    }
    if (fgets(line, LINE_SIZE, f) == NULL) {
        printf("Could not read file\n");
        exit(-1);
    }

    if (!feof(f)) {
        printf("Program line too long ...\n");
        exit(-1);
    }

    token = strtok(line, sep);
    while (token != NULL && i < PROGRAM_SIZE) {
        program[i++] = atoi(token);
        token = strtok(NULL, sep);
    }
    printf("Program is %d %d %d %d ...\n", program[0], program[1], program[2], program[3]);
}


int main(void) {
    int position = 0;
    int16_t program[PROGRAM_SIZE];

    parse_program("day_2.input", program);
    
    while(position >= 0)
       position = run(program, position);

    return 0;
}


