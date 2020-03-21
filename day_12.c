#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SIGN(x) (((x) > 0) - ((x) < 0))
#define ABS(x) (((x) > 0) ? (x) : -(x))
typedef struct {
    int x;
    int y;
    int z;
} Vector;

static inline void add(Vector *c, const Vector *a, const Vector *b)
{
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    c->z = a->z + b->z;
}

static inline void neg(Vector *c, const Vector *a)
{
    c->x = -a->x;
    c->y = -a->y;
    c->z = -a->z;
}

static inline int get_abs(Vector *a)
{
    return ABS(a->x) + ABS(a->y) + ABS(a->z);
}

static inline void print_vec(Vector *a, int newline)
{
    printf("<x=%d, y=%d, z=%d>", a->x, a->y, a->z);
    if (newline)
        printf("\n");
}

typedef struct {
    Vector pos;
    Vector vel;
} Body;


/* get acceleration for Body a when it interacts with b */
static void get_accel(Vector *acc, Body *a, Body *b)
{
    acc->x = SIGN(b->pos.x - a->pos.x);
    acc->y = SIGN(b->pos.y - a->pos.y);
    acc->z = SIGN(b->pos.z - a->pos.z);
}

static void gravity_step(Body *moons, int n_moons)
{
    int i,j;
    Vector acc;

    for (i = 0; i < n_moons; i++) {
        for (j = i+1; j < n_moons; j++) {
            get_accel(&acc, &moons[i], &moons[j]);
            add(&moons[i].vel, &moons[i].vel, &acc);
            neg(&acc, &acc);
            add(&moons[j].vel, &moons[j].vel, &acc);
        }
    }
}

static void velocity_step(Body *moons, int n_moons)
{
    int i;
    for (i = 0; i < n_moons; i++) {
        add(&moons[i].pos, &moons[i].pos, &moons[i].vel);
    }
}

static void print_moons(Body *moons, int n_moons)
{
    int i;
    for (i = 0; i < n_moons; i++) {
        printf("pos=");
        print_vec(&moons[i].pos, 0);
        printf(", vel=");
        print_vec(&moons[i].vel, 1);
    }
    printf("\n");
}

static int get_energy(Body *moons, int n_moons)
{
    int i;
    int energy = 0;

    for (i = 0; i < n_moons; i++) {
        energy += get_abs(&moons[i].pos) * get_abs(&moons[i].vel);
    }

    return energy;
}

static int parse_initial_position(Body *moons, const char *filename)
{
    FILE *f = fopen(filename, "r");
    char *line = NULL;
    char *p;
    size_t size = 0;
    int i = 0;
    Body *b = moons;
    Vector *pos;

    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }

    while (getline(&line, &size, f) >= 0) {
        memset(b, 0, sizeof(*b));
        pos = &b->pos;
        p = strchr(line, '=') + 1;
        pos->x = atoi(p);
        p = strchr(p, '=') + 1;
        pos->y = atoi(p);
        p = strchr(p, '=') + 1;
        pos->z = atoi(p);
        b++;
        i++;
    }

    printf ("%d bodies initialized\n", i);
    return i;
}


int main(int argc, char **argv)
{
    Body moons[4];
    int n_moons;
    int i, n_steps = atoi(argv[2]);
    int energy;
    
    n_moons = parse_initial_position(moons, argv[1]);
    if (n_moons != 4)
        return -1;

    for (i = 0; i < n_steps; i++) {
        gravity_step(moons, n_moons);
        velocity_step(moons, n_moons);
    }
    energy = get_energy(moons, n_moons);
    printf("Total energy %d\n", energy);

    return 0;
}
