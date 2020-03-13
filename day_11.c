
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "intcode.h"

#define PROGRAM_SIZE 640
#define GRID_SIZE 96
#define INPUT_SIZE 32

#define PAINT_BIT (1 << 7)
uint8_t grid[GRID_SIZE * GRID_SIZE];

enum {
    ROBOT_UP,
    ROBOT_RIGHT,
    ROBOT_DOWN,
    ROBOT_LEFT
};

typedef struct {
    int x;
    int y;
    int dir;
    int cur_prog_pos;
    int n_painted_once;
    Context brain;
} Robot;

void init_grid(void)
{
    memset(grid, 0, sizeof(grid));
    /* Step 2 */
    grid[(GRID_SIZE/2) * GRID_SIZE + (GRID_SIZE/2)] = 1;
}

void init_robot(Robot *r, int64_t *program)
{
    r->x = 0;
    r->y = 0;
    r->cur_prog_pos = 0;
    r->dir = ROBOT_UP;
    r->n_painted_once = 0;
    r->brain.program = program;
    r->brain.relative_base = 0;
    r->brain.input = malloc(INPUT_SIZE * sizeof(int64_t));
    r->brain.output = malloc(2*sizeof(int64_t));
}

void free_robot(Robot *r)
{
    free(r->brain.input);
    free(r->brain.output);
}

void step_robot(Robot *r)
{
    switch (r->dir) {
        case ROBOT_UP:
            (r->y)--;
            break;
        case ROBOT_RIGHT:
            (r->x)++;
            break;
        case ROBOT_DOWN:
            (r->y)++;
            break;
        case ROBOT_LEFT:
            (r->x)--;
            break;
    }
    if (abs(r->x) >= GRID_SIZE/2 || abs(r->y) >= GRID_SIZE/2) {
        printf("Robot out of the grid (%d,%d)\n", r->x, r->y);
        exit(-1);
    }
}

void run_robot(Robot *r)
{
    int x, y, i;
    for(;;) {
        x = GRID_SIZE/2 + r->x;
        y = GRID_SIZE/2 + r->y;

        r->brain.input_idx = 0;
        r->brain.output_idx = 0;
        /* The robot can use several input instructions even for a single step */
        for (i = 0; i < INPUT_SIZE; i++) {
            r->brain.input[i] = (grid[y * GRID_SIZE + x] & 0x7F);
        }
        
        /* Let the robot produce two outputs */
        r->cur_prog_pos = resume_till_output(&r->brain, r->cur_prog_pos);
        if (r->cur_prog_pos < 0)
            break;
        r->cur_prog_pos = resume_till_output(&r->brain, r->cur_prog_pos);
       
       /* Output 1 : color */
        if ((grid[y * GRID_SIZE + x] & PAINT_BIT) == 0)
            r->n_painted_once++;
        
        grid[y * GRID_SIZE + x] = PAINT_BIT | (r->brain.output[0] & 0x7F);

        /* Output 2 : rotation then move */
        r->dir = (4 + r->dir + (r->brain.output[1]*2 - 1)) % 4;
        step_robot(r);
    }
}

void display_grid(void)
{
    int i, j;
    for (i = 0; i < GRID_SIZE - 1; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            if ((grid[i*GRID_SIZE + j] & 0x7F))
                printf("-");
            else
                printf(" ");
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int64_t *program;
    Robot robot;
    int size;
    
    program = malloc(PROGRAM_SIZE * sizeof(int64_t));
    size = parse_program(argv[1], program);
    if (size > PROGRAM_SIZE) {
        printf("Program too big %d\n", size);
        return -1;
    }

    init_grid();
    init_robot(&robot, program);
    run_robot(&robot);
    printf("Number of panels painted once %d\n", robot.n_painted_once);
    
    display_grid();
    
    free_robot(&robot);
    free(program);

    return 0;
}
