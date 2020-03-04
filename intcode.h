
#include <inttypes.h>

typedef struct {
    int64_t *program;
    int64_t *input;
    int input_idx;
    int64_t *output;
    int output_idx;
    int pause_on_output;
    int relative_base;
} Context;

/* Run the entier program, return program[0] */
int run(Context *ctx);

/* Resume program from cur_pos until next program output. Returns current position */
int resume_till_output(Context *ctx, int cur_pos);

/* Parse file to extract a program, to be filled in Context.program, return program size */
int parse_program(const char *filename, int64_t *program);
