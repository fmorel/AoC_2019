
#include <inttypes.h>

typedef struct {
    int32_t *program;
    int *input;
    int input_idx;
    int *output;
    int output_idx;
} Context;

int run(Context *ctx);
int parse_program(const char *filename, int32_t *program);
