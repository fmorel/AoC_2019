
#include <inttypes.h>

typedef struct {
    int64_t *program;
    int64_t *input;
    int input_idx;
    int64_t *output;
    int output_idx;
    int output_triggered;
    int pause_on_input;
    int relative_base;
} Context;

/* Run the entier program, return program[0] */
int run(Context *ctx);

/* Run a signle instruction */
int run_inst(Context *ctx, int position);
/* Resume program from cur_pos until next program output. Returns current position */
int resume_till_output(Context *ctx, int cur_pos);

/* Resume until next event depending on *event value(after output and/or before input).
 * Return current position and modifies event */
#define INTCODE_EVENT_INPUT  (1<<0)
#define INTCODE_EVENT_OUTPUT (1<<1)
int resume_till_event(Context *ctx, int *event, int cur_pos);

/* Parse file to extract a program, to be filled in Context.program, return program size */
int parse_program(const char *filename, int64_t *program);
