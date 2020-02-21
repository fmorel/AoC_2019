#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


typedef struct Body {
    uint32_t name;  /* 3 octets */
    int m_idx;      //Master body idx or -1 or NULL
    int n_orbits;   //Total number of direct/indirect orbits
} Body;

typedef struct {
    Body *bodies;
    int n_bodies;
    int n_bodies_alloc;
} Universe;

static Body *find_body(const Universe *u, uint32_t name)
{
    int i;
    for (i=0; i < u->n_bodies; i++) {
        if (u->bodies[i].name == name)
            return &u->bodies[i];
    }
    return NULL;
}

static uint32_t get_name(char *str)
{
    return (str[2] << 16) | (str[1] << 8) | str[0];
}

static int get_idx(Universe *u, Body *b)
{
    return (b - u->bodies);
}

static Body *get_master(Universe *u, Body *b)
{
    return &u->bodies[b->m_idx];
}

static int *get_master_chain(Universe *u, int idx)
{
    int *ret;
    Body *b = &u->bodies[idx];
    int i, n;

    n = b->n_orbits;
    ret = malloc(n * sizeof(int));
    for (i = 0; i < n; i++) {
        ret[i] = b->m_idx;
        b = get_master(u, b);
    }
    return ret;
}

static int find_common_ancestor(Universe *u, int *chain1, int len1, int *chain2, int len2)
{
    int i, j;
    int small_len, *small_chain, big_len, *big_chain;

    if (len1 < len2) {
        small_len = len1;
        small_chain = chain1;
        big_len = len2;
        big_chain = chain2;
    } else {
        small_len = len2;
        small_chain = chain2;
        big_len = len1;
        big_chain = chain1;
    }

    for (i = 0; i < small_len; i++) {
        for (j = i; j < big_len; j++) {
            if (big_chain[j] == small_chain[i])
                goto found;
        }
    }

found: 
    if (big_chain[j] == small_chain[i])
        return big_chain[j];

    return -1;
}

static Body *add_body(Universe *u, Body *b)
{
    if (u->n_bodies > (3 * u->n_bodies_alloc)/4) {
        u->n_bodies_alloc *= 2;
        u->bodies = realloc(u->bodies, u->n_bodies_alloc * sizeof(Body));
    }
    Body *b2 = &u->bodies[u->n_bodies];
    *b2 = *b;
    printf("Add body %s\n", (char*)&b2->name);
    u->n_bodies++;
    return b2;
}

static int count_orbits_inner(Universe *u, int idx)
{
    Body *b = &u->bodies[idx];
    
    if (b->n_orbits < 0) {
        if (b->m_idx >= 0)
            b->n_orbits = 1 + count_orbits_inner(u, b->m_idx);
        else
            b->n_orbits = 0;
    }
    return b->n_orbits;
}

int parse_orbits(const char *filename, Universe *u)
{
    int i, j =0;
    FILE *f = fopen(filename, "r");
    char *line = NULL;
    size_t line_size = 0;
    Body *b1, *b2, b_s;
    uint32_t name1, name2;

    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }

    u->bodies = malloc(16 * sizeof(Body));
    u->n_bodies = 0;
    u->n_bodies_alloc = 16;

    while (getline(&line, &line_size, f) > 0) {
        name1 = get_name(line);
        name2 = get_name(&line[4]);

        /* Find B1 or add it if it doesn't exist */
        b1 = find_body(u, name1);
        if (!b1) {
            b_s.name = name1;
            b_s.m_idx = -1;
            b_s.n_orbits = -1;
            b1 = add_body(u, &b_s);
        }
        /* If B2 already exists, find it and update master's body
         * Or else just adds it */
        b2 = find_body(u, name2);
        if (b2) {
            if (b2->m_idx >= 0) {
                printf("Error, Body %d is already orbiting %d\n", name2, u->bodies[b2->m_idx].name);
                return -1;
            }
            b2->m_idx = get_idx(u, b1);
        } else {
            b_s.name = name2;
            b_s.m_idx = get_idx(u, b1);
            b_s.n_orbits = -1;
            b2 = add_body(u, &b_s);
        }
    }
    printf("Universe parsed : %d bodies found\n", u->n_bodies);
    return u->n_bodies;
}




int main(int argc, char **argv)
{
    int i, n, n_orbits;
    Universe u;

    n = parse_orbits(argv[1], &u);
    
    n_orbits = 0;
    for (i = 0; i < n; i++) {
        if (u.bodies[i].m_idx < 0)
            printf("Body %s has no master\n", (char*)&u.bodies[i].name);
        n_orbits += count_orbits_inner(&u, i);
    }
    printf("Total orbits is %d\n", n_orbits);

    Body *you, *san, *common;
    int *you_chain, *san_chain;
    int common_idx;
    
    you = find_body(&u, get_name("YOU"));
    san = find_body(&u, get_name("SAN"));
    you_chain = get_master_chain(&u, get_idx(&u, you));
    san_chain = get_master_chain(&u, get_idx(&u, san));
    common_idx = find_common_ancestor(&u, you_chain, you->n_orbits, san_chain, san->n_orbits);
    free(you_chain);
    free(san_chain);
    if (common_idx < 0) {
        printf("Error : no common ancestor between YOU and SAN \n");
        exit(1);
    }
    common = &u.bodies[common_idx];
    n_orbits = you->n_orbits + san->n_orbits - 2 * common->n_orbits - 2;
    printf("Minimal transfer is %d hops\n", n_orbits);

    free(u.bodies);
    return 0;
}
