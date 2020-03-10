#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int x;
    int y;
    int n_detect;
} Asteroid;

typedef struct {
    int n_asts;
    Asteroid *asts;
    int width;
    int height;
} AsteroidField;

static void add_asteroid(AsteroidField *f, int x, int y)
{
    Asteroid *a;
    f->asts = realloc(f->asts, (f->n_asts + 1) * sizeof(Asteroid));

    a = &f->asts[f->n_asts++];
    a->x = x;
    a->y = y;
}

/* Basic Euclid's algorithm to fetch unsigned GCD between a and b */
static int gcd (int a, int b)
{
    int temp;
    /* Unsign the GCD !*/
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    if (b > a) {
        temp = a;
        a = b;
        b = temp;
    }
    while (b != 0) {
        temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

/* Get the smallest vector defining line of sight from Asteroid a to Asteroid b */
static void get_los_unit_vector(const Asteroid *a, const Asteroid *b, int *vx, int *vy)
{
    int dx, dy, g;
    dx = b->x - a->x;
    dy = b->y - a->y;
    
    g = gcd(dx, dy);
    *vx = dx / g;
    *vy = dy / g;
}

#define ASTEROID_BIT 0
#define MASK_BIT 1
#define CENTER_BIT 2
/* Represent asteroid field with a bitmap containing relevant information 
 * Returns 0 if coordiantes are outside the field*/
static int mask_point(const AsteroidField *f, uint8_t *field_bmp, int x, int y)
{
    if ((x >= f->width) || (x < 0) || (y >= f->height) || (y < 0))
        return 0;
    field_bmp[y * f->width + x] |= (1 << MASK_BIT);
    return 1;
}

static int is_masked(const AsteroidField *f, const Asteroid *a, const uint8_t *field_bmp)
{
    return ((field_bmp[a->y * f->width + a->x] >> MASK_BIT) & 1);
}

void dump_field(const AsteroidField *f, uint8_t *field_bmp)
{
    int i,j;
    uint8_t val;
    for (i = 0; i < f->height; i++) {
        for (j = 0; j < f->width; j++) {
            val = field_bmp[i * f->width + j];
            if (val == 0) {
                printf(" ");
                continue;
            }
            if (val & (1<<CENTER_BIT))
                printf("o");
            else if (val & (1<<MASK_BIT)) {
                if (val & (1<<ASTEROID_BIT))
                    printf("x");
                else
                    printf("-");
            } else if (val & (1<<ASTEROID_BIT))
                printf("*");
        }
        printf("\n");
    }
}

static int examine_asteroid(AsteroidField *f, int idx, int do_dump)
{
    int i, vx, vy, px, py;
    Asteroid *a = &f->asts[idx];
    uint8_t *field_bmp;

    /* Use 64 bit for a line of asteroid field since width < 64 */
    field_bmp = calloc(f->height * f->width, 1);
    field_bmp[a->y * f->width + a->x] = (1 << CENTER_BIT);

    /* Scan all the asteroids and create mask */
    for (i = 0; i < f->n_asts; i++) {
        if (i == idx)
            continue;
        field_bmp[f->asts[i].y * f->width + f->asts[i].x] |= (1 << ASTEROID_BIT);
        get_los_unit_vector(a, &f->asts[i], &vx, &vy);
        /* Mask all the points behind asteroid i in the line of sight */
        px = f->asts[i].x + vx;
        py = f->asts[i].y + vy;
        while (mask_point(f, field_bmp, px, py)) {
            px += vx;
            py += vy;
        }
    }
    
    /* Count the unmasked asteroids */
    a->n_detect = 0;
    for (i = 0; i < f->n_asts; i++) {
        if (i == idx)
            continue;
        if (is_masked(f, &f->asts[i], field_bmp))
            continue;
        a->n_detect++;
    }
    
    if (do_dump)
        dump_field(f, field_bmp);
    free(field_bmp);
    return a->n_detect;
}

static int parse_asteroids(const char *filename, AsteroidField *field)
{
    FILE *f = fopen(filename, "r");
    char *line = NULL;
    size_t line_size = 0;
    int i, n_lines = 0, width; 
    
    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }
    
    memset(field, 0, sizeof(*field));

    while (getline(&line, &line_size, f) > 0) {
        for(i = 0; i < line_size; i++) {
            if (line[i] == '#') {
                add_asteroid(field, i, n_lines);
            }
            if (line[i] == '\n') {
                width = i;
                break;
            }
        }
        n_lines++;
    }
    free(line);
    field->width = width;
    field->height = n_lines;

    printf("%d asteroids found in a field of size %dx%d\n", field->n_asts, field->width, field->height);
}

void debug(const AsteroidField *f)
{
    int i;
    for (i = 0; i < f->n_asts; i++) {
        Asteroid *a = &f->asts[i];
        printf("Asteroid %d (%d,%d). n_detect = %d\n",
            i, a->x, a->y, a->n_detect);
    }
}

int main(int argc, char **argv)
{
    AsteroidField field;
    int i, n, i_max, n_max = 0;

    parse_asteroids(argv[1], &field);
    if (field.width >= 64) {
        printf("Program only supports a field width of max 64 (%d)\n", field.width);
        return -1;
    }

    /* Step 1 */
    for (i = 0; i < field.n_asts; i++) {
        n = examine_asteroid(&field, i, 0);
        if (n > n_max) {
            n_max = n;
            i_max = i;
        }
    }
    printf("Best asteroid is %d at (%d,%d) with %d detected asteroids\n",
        i_max, field.asts[i_max].x, field.asts[i_max].y, n_max);
    //debug(&field);
    examine_asteroid(&field, i_max, 1);

    free(field.asts);
    return 0;
}
