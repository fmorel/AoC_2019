#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intcode.h"

#define PROGRAM_SIZE 528

typedef struct {
    int settings[120][5];
    int cur_idx;
} PhaseSettings;

void alloc_amplifier(Context *amp, int prog_size)
{
    amp->input = malloc(2 * sizeof(int));
    amp->output = malloc(1 * sizeof(int));
    amp->program = malloc(prog_size * sizeof(int32_t));
}

int run_amplifiers(Context *amp, int32_t *prog, int prog_size, PhaseSettings *s, int idx)
{
    int i;
    int *phase = s->settings[idx];
    int inout_value = 0;
    for (i = 0; i < 5; i++) {
        /* Init amplifier */
        amp->input_idx = 0;
        amp->input[0] = phase[i];
        amp->input[1] = inout_value;
        amp->output_idx = 0;
        memcpy(amp->program, prog, prog_size * sizeof(int32_t));
        /* Run */
        run(amp);
        inout_value = amp->output[0];
    }
    return inout_value;
}

/* Heap's algorithm to generate permutations */

void swap (int *x, int *y)
{
    int temp;

    temp = *x;
    *x = *y;
    *y = temp;
}

void store_setting(const int *v, PhaseSettings *s)
{
    memcpy(s->settings[s->cur_idx++], v, 5 * sizeof(int));
}

void dump_setting(PhaseSettings *s, int idx)
{
    printf ("%d %d %d %d %d\n",
            s->settings[idx][0], s->settings[idx][1],
            s->settings[idx][2], s->settings[idx][3],
            s->settings[idx][4]);
} 

void dump_settings(PhaseSettings *s)
{
    int i;

    for (i = 0; i < 120; i++) {
        dump_setting(s, i);
    }
}

void heappermute(int *v, int n, PhaseSettings *s) {

    int i;

    if (n == 1) {
        store_setting(v, s);
    } else {
        for (i = 0; i < n; i++) {
            heappermute(v, n-1, s);
            if (n % 2 == 1) {
                swap(&v[0], &v[n-1]);
	    }
            else {
                swap(&v[i], &v[n-1]);
            }
	}
    }
}

int main(int argc, char **argv)
{
    int32_t program[PROGRAM_SIZE];
    int size, i;
    Context amp;
    PhaseSettings *s;
    int v_init[5] = {0, 1, 2, 3, 4};
    int idx_max, out_max, out;

    /* Allocate 5 instances of IntCode contexts */
    size = parse_program(argv[1], program);
    
    alloc_amplifier(&amp, size);

    /* Allocate and generate PhaseSetting */
    s = malloc(sizeof(PhaseSettings));
    s->cur_idx = 0;
    heappermute(v_init, 5, s);
    //dump_settings(s);

    /* Run amplifiers for each phase setting */
    out_max = 0;
    for (i = 0; i < 120; i++) {
        out = run_amplifiers(&amp, program, size, s, i); 
        if (out > out_max) {
            out_max = out;
            idx_max = i;
        }
    }

    printf("Max output is %d with setting ", out_max);
    dump_setting(s, idx_max);
}
