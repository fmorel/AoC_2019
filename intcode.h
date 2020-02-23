
#include <inttypes.h>

typedef struct {
    int32_t *program;
    int *input;
    int input_idx;
    int *output;
    int output_idx;
    int pause_on_output;
} Context;

/* Run the entier program, return program[0] */
int run(Context *ctx);

/* Resume program from cur_pos until next program output. Returns current position */
int resume_till_output(Context *ctx, int cur_pos);

/* Parse file to extract a program, to be filled in Context.program, return program size */
int parse_program(const char *filename, int32_t *program);
