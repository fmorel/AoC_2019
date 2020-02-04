#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define WIRE_SIZE 512

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point a;
    Point b;
    int direction;
    int steps;
} Segment;

#define HORIZ_MASK 1
#define VERT_MASK (1 << 1)
#define DIR_MASK (HORIZ_MASK | VERT_MASK)

enum {
    RIGHT =  HORIZ_MASK,
    UP =    VERT_MASK,
    LEFT =  (1 << 2) | HORIZ_MASK,
    DOWN =  (1 << 2) | VERT_MASK
};


typedef struct {
    Segment segments[WIRE_SIZE];
    int size;
} Wire;

int segment_cross(const Segment *s1, const Segment *s2, Point *inter)
{
    const Segment *h, *v;
    int h_steps, v_steps;
    /* Only segments in opposite direction can intersect */
    if (s1->direction & s2->direction & DIR_MASK != 0)
        return 0;
    if (s1->direction & HORIZ_MASK) {
        h = s1;
        v = s2;
    } else {
        h = s2;
        v = s1;
    }
    if (h->a.x <= v->a.x && h->b.x >= v->a.x &&
        v->a.y <= h->a.y && v->b.y >= h->a.y) {

        inter->x = v->a.x;
        inter->y = h->a.y;
        if (h->direction == RIGHT)
            h_steps = inter->x - h->a.x;
        else
            h_steps = h->b.x - inter->x;

        if (v->direction == UP)
            v_steps = inter->y - v->a.y;
        else
            v_steps = v->b.y - inter->y;

        return ((h->steps + h_steps) + (v->steps + v_steps)) ;
    }
    return 0;
}

void wire_cross(const Wire *wires)
{
    int i, j;
    Point inter;
    int min_dist = INT_MAX, dist, steps, min_steps = INT_MAX;

    for (i = 0; i < wires[0].size; i++) {
        for (j = 0; j < wires[1].size; j++) {
            steps = segment_cross(&wires[0].segments[i], &wires[1].segments[j], &inter);
            if (steps) {
                dist = abs(inter.x) + abs(inter.y);
                if (dist != 0 && dist < min_dist)
                    min_dist = dist;
                if (dist != 0 && steps < min_steps)
                    min_steps = steps;
            }
        }
    }
    printf("min_dist is %d\n", min_dist);
    printf("min_steps is %d\n", min_steps);
}

int parse_wires(const char *filename, Wire *wires)
{
    int i, j =0;
    FILE *f = fopen(filename, "r");
    char *token = NULL, *line = NULL;
    size_t line_size = 0;
    const char sep[2] = ",";
    char dir;
    int len, steps;
    Point pos;

    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }
    while (getline(&line, &line_size, f) > 0) {
        token = line;
        j = 0;
        pos.x = 0; pos.y = 0;
        steps = 0;
        while (j < WIRE_SIZE) {
            dir = token[0];
            len = atoi(&token[1]);
            Segment *seg = &wires[i].segments[j];
            /* Store the segment so that point a is always lower than point b */
            switch (dir) {
                case 'R':
                    seg->a = pos;
                    pos.x += len;
                    seg->b = pos;
                    seg->direction = RIGHT;
                    break;
                case 'U':
                    seg->a = pos;
                    pos.y += len;
                    seg->b = pos;
                    seg->direction = UP;
                    break;
                case 'L':
                    seg->b = pos;
                    pos.x -= len;
                    seg->a = pos;
                    seg->direction = LEFT;
                    break;
                case 'D':
                    seg->b = pos;
                    pos.y -= len;
                    seg->a = pos;
                    seg->direction = DOWN;
                    break;
                default:
                    printf("Unknown direction\n");
                    return -1;
            }
            seg->steps = steps;
            steps += len;
            j++;
            token = strchr(token, ',');
            if (!token)
                break;
            token++;
        }
        wires[i].size = j;
        printf("Wire size %d\n", j);
        i++;
    }
    free(line);
    printf("Wires number %d\n", i);
    return 0;
}

int main(int argc, char **argv) {
    static Wire wire[2];
    int size;

    parse_wires(argv[1], wire);
    wire_cross(wire);
    return 0;
}
