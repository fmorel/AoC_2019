#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intcode.h"

#define PROGRAM_SIZE 528
#define FEEDBACK_N_LOOP 16

typedef struct {
    int settings[120][5];
    int cur_idx;
} PhaseSettings;


void alloc_amplifier(Context *amp, int prog_size)
{
    /* Init bigger in/out spaces for step 2 */
    amp->input = malloc(FEEDBACK_N_LOOP * sizeof(int));
    amp->output = malloc(FEEDBACK_N_LOOP * sizeof(int));
    amp->program = malloc(prog_size * sizeof(int32_t));
}

void init_amplifier(Context *amp, int32_t *prog, int prog_size, int *phase, int amp_idx)
{
    amp->input_idx = 0;
    amp->input[0] = phase[amp_idx];
    amp->output_idx = 0;
    memcpy(amp->program, prog, prog_size * sizeof(int32_t));
}

int run_amplifiers(Context *amp, int32_t *prog, int prog_size, PhaseSettings *s, int idx)
{
    int i;
    int *phase = s->settings[idx];
    int inout_value = 0;

    for (i = 0; i < 5; i++) {
        /* Init amplifier */
        init_amplifier(amp, prog, prog_size, phase, i);
        amp->input[1] = inout_value;
        /* Run */
        run(amp);
        inout_value = amp->output[0];
    }
    return inout_value;
}

int run_amplifiers_feedback(Context *amps, int32_t *prog, int prog_size, PhaseSettings *s, int idx)
{
    int *phase = s->settings[idx];
    int inout_value = 0;
    int amp_cur_pos[5];
    int i, n_loop = 0;
    Context *amp;
    
    /* Set amplifiers to program start */
    memset(amp_cur_pos, 0, sizeof(amp_cur_pos));
    
    for (i = 0; i < 5; i++) {
        init_amplifier(&amps[i], prog, prog_size, phase, i);
    }

    for (i = 0; i < 5; i++) {
        amp = &amps[i];
        /* After a first run, each amplifier has updated its input_idx to the next available input */
        if (n_loop == 0)
            amp->input[1] = inout_value;
        else
            amp->input[amp->input_idx] = inout_value;
        /* Run amplifier to next output instruction and store resume address */
        amp_cur_pos[i] = resume_till_output(amp, amp_cur_pos[i]);
        /* Last output is at output_idx-1 */
        inout_value = amp->output[amp->output_idx - 1];
        
        /* Feedback loop to amplifier A if amplifier E has not finished 
         * Rerun the for-loop */
        if (i == 4 && amp_cur_pos[i] >= 0) {
            i = -1;
            n_loop++;
        }
    }

    if (n_loop >= FEEDBACK_N_LOOP) {
       printf("Too many feedback loops (%d)\n", n_loop);
       exit(1);
    }

    return inout_value;
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

/* Heap's algorithm to generate permutations */
void swap (int *x, int *y)
{
    int temp;

    temp = *x;
    *x = *y;
    *y = temp;
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

void step1(int32_t *program, int size)
{
    Context amp;
    PhaseSettings *s;
    int v_init[5] = {0, 1, 2, 3, 4};
    int i, idx_max, out_max, out;

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

void step2(int32_t *program, int size)
{
    Context amps[5];
    PhaseSettings *s;
    int v_init[5] = {5, 6, 7, 8, 9};
    int i, idx_max, out_max, out;
    
    /* Alloc 5 amplifiers */
    for (i = 0; i < 5; i++) {
        alloc_amplifier(&amps[i], size);
    }

    /* Allocate and generate PhaseSetting */
    s = malloc(sizeof(PhaseSettings));
    s->cur_idx = 0;
    heappermute(v_init, 5, s);
    
    out_max = 0;
    for (i = 0; i < 120; i++) {
        out = run_amplifiers_feedback(amps, program, size, s, i); 
        if (out > out_max) {
            out_max = out;
            idx_max = i;
        }
    }

    printf("Max output is %d with setting ", out_max);
    dump_setting(s, idx_max);
}


int main(int argc, char **argv)
{
    int32_t program[PROGRAM_SIZE];
    int size;
    
    size = parse_program(argv[1], program);
    
    /*
    printf("Step1\n----------\n");
    step1(program, size);
    */

    printf("Step2\n----------\n");
    step2(program, size);
}
