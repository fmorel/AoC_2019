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
} Segment;

enum {
    HORIZONTAL,
    VERTICAL
};

typedef struct {
    Segment segments[WIRE_SIZE];
    int size;
} Wire;

int segment_cross(const Segment *s1, const Segment *s2, Point *inter)
{
    const Segment *h, *v;
    if (s1->direction == s2->direction)
        return 0;
    if (s1->direction == HORIZONTAL) {
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
        return 1;
    }
    return 0;
}

void wire_cross(const Wire *wires)
{
    int i, j;
    Point inter;
    int min_dist = INT_MAX, dist;

    for (i = 0; i < wires[0].size; i++) {
        for (j = 0; j < wires[1].size; j++) {
            if (segment_cross(&wires[0].segments[i], &wires[1].segments[j], &inter)) {
                dist = abs(inter.x) + abs(inter.y);
                if (dist != 0 && dist < min_dist)
                    min_dist = dist;
            }
        }
    }
    printf("min_dist is %d\n", min_dist);
}

int parse_wires(const char *filename, Wire *wires)
{
    int i, j =0;
    FILE *f = fopen(filename, "r");
    char *token = NULL, *line = NULL;
    size_t line_size = 0;
    const char sep[2] = ",";
    char dir;
    int len;
    Point pos;

    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }
    while (getline(&line, &line_size, f) > 0) {
        token = line;
        j = 0;
        pos.x = 0; pos.y = 0;
        while (j < WIRE_SIZE) {
            dir = token[0];
            len = atoi(&token[1]);
            Segment *seg = &wires[i].segments[j];
            printf("Segment dir %c len %d\n", dir, len);
            switch (dir) {
                case 'R':
                    seg->a = pos;
                    pos.x += len;
                    seg->b = pos;
                    seg->direction = HORIZONTAL;
                    break;
                case 'U':
                    seg->a = pos;
                    pos.y += len;
                    seg->b = pos;
                    seg->direction = VERTICAL;
                    break;
                case 'L':
                    seg->b = pos;
                    pos.x -= len;
                    seg->a = pos;
                    seg->direction = HORIZONTAL;
                    break;
                case 'D':
                    seg->b = pos;
                    pos.y -= len;
                    seg->a = pos;
                    seg->direction = VERTICAL;
                    break;
                default:
                    printf("Unknown direction\n");
                    return -1;
            }
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
