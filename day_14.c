#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_REACTANTS 8

typedef struct {
    uint8_t qty;
    uint8_t idx;    /* Index in Product array */
} Reactant;

typedef struct {
    uint8_t qty;
    uint8_t n_reactants;
    Reactant reactants[MAX_REACTANTS];
} Reaction;

typedef union {
    uint64_t i;
    char s[8];
} ProductID;

typedef struct {
    ProductID id;
    Reaction reaction;
    int needed;
    int produced;
} Product;

typedef struct {
    int n_products;
    int n_alloc;
    Product *products;
} Chemistry;

static int add_product(Chemistry *c, ProductID id)
{
    if (c->n_products >= (3 * c->n_alloc)/4) {
        c->n_alloc *= 2;
        c->products = realloc(c->products, c->n_alloc * sizeof(Product));
    }
    Product *p = &c->products[c->n_products];
    memset(p, 0, sizeof(Product));
    p->id = id;
    c->n_products++;
    return (c->n_products - 1);
}

static int find_product(Chemistry *c, ProductID id)
{
    int i;
    for (i = 0; i < c->n_products; i++) {
        if (c->products[i].id.i == id.i)
            return i;
    }
    return -1;
}

void parse_product(ProductID *id, uint8_t *qty, const char *token, size_t token_size)
{
    char *p;
    size_t size;

    /* First parse integer to get qty */
    *qty = atoi(token);
    /* Then parse name and directly convert to ProductID */
    p = strchr(token, ' ') + 1;
    size = token_size - (p-token);
    id->i = 0;
    memcpy(id->s, p, size);
}

void parse_and_add_reactant(Chemistry *c, Reaction *r, const char *token, size_t token_size)
{
    int idx;
    uint8_t qty;
    ProductID id;
    Reactant *reactant;
    
    parse_product(&id, &qty, token, token_size);

    idx = find_product(c, id); 
    if (idx < 0) {
        idx = add_product(c, id); 
    }
    /* Add reactant */
    if (r->n_reactants == MAX_REACTANTS)
        exit(-1);
    reactant = &r->reactants[r->n_reactants++];
    reactant->idx = idx;
    reactant->qty = qty;
}

void print_reactions(Chemistry *c)
{
    int i, j;
    Product *p;

    for (i = 0; i < c->n_products; i++) {
        p = &c->products[i];
        for (j = 0; j < p->reaction.n_reactants; j++) {
            Reactant *r = &p->reaction.reactants[j];
            printf("%d %s(%d), ", r->qty, c->products[r->idx].id.s, r->idx);
        }
        printf(" => %d '%s'(%d)\n", p->reaction.qty, p->id.s, i);
    }
}


int parse_reactions(Chemistry *c, const char *filename)
{
    char *line = NULL;
    size_t line_size = 0;
    char *p1, *p2;
    Product *product;
    ProductID id;
    Reaction reaction;
    int idx;
    uint8_t qty;
    FILE *f = fopen(filename, "r");
    
    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }
    
    memset(c, 0, sizeof(*c));
    c->n_alloc = 4;
    c->products = malloc(4 * sizeof(Product));

    while (getline(&line, &line_size, f) > 0) {
        /* Init reaction */
        p1 = line;
        reaction.n_reactants = 0;

        /* Parse first reactants (if any) */
        while (p2 = strchr(p1, ',')) {
            parse_and_add_reactant(c, &reaction, p1, (p2-p1));
            p1 = p2 + 2;
        }
        /* Parse last reactant */
        p2 = strchr(p1, '=');
        parse_and_add_reactant(c, &reaction, p1, (p2-p1-1));
        
        /* Jump over the "=> " token */
        p1 = p2 + 3;
        /* Parse reaction product */
        parse_product(&id, &qty, p1, strlen(line) - 1 - (p1 - line));     
        idx = find_product(c, id); 
        if (idx < 0) {
            idx = add_product(c, id); 
        }
        /* Copy reaction content */
        product = &c->products[idx];
        reaction.qty = qty;
        product->reaction = reaction;
    }

    fclose(f);
    return c->n_products;
}

int produce(Chemistry *c, Product *p)
{
    int i, n;
    Reaction *r;
    Reactant *re;

    n = p->needed - p->produced;
    if (n <= 0)
        return 0;

    r = &p->reaction;
    if (!r->qty)
        return 0;
    /* Get the number of reactions to run */
    n = (n + r->qty - 1) / r->qty;
    for (i = 0; i < r->n_reactants; i++) {
        re = &r->reactants[i];
        c->products[re->idx].needed += re->qty * n;
    }
    p->produced += n * r->qty;
    return 1;
}

void compute_reactions(Chemistry *c)
{
    ProductID fuel_id, ore_id;
    Product *fuel;
    int idx, i;
    int need_to_run, loop, ore_qty;

    fuel_id.i = 0;
    strcpy(fuel_id.s, "FUEL");
    ore_id.i = 0;
    strcpy(ore_id.s, "ORE");

    idx = find_product(c, fuel_id);
    if (idx < 0) {
        printf("FUEL not found\n");
        exit(1);
    }
    c->products[idx].needed = 1;
    need_to_run = 1;
    loop = 0;
    while (need_to_run) {
        need_to_run = 0;
        for (i = 0; i < c->n_products; i++) {
            if (produce(c, &c->products[i]))
                need_to_run = 1;
        }
        loop++;
    }

    idx = find_product(c, ore_id);
    if (idx < 0) {
        printf("ORE not found\n");
        exit(1);
    }

    ore_qty = c->products[idx].needed;
    printf("Fuel produced after %d loops, need %d ore\n",
            loop, ore_qty);
}


int main (int argc, char **argv)
{
    Chemistry c;
    int n_products = parse_reactions(&c, argv[1]);
    
    printf("%d reactions found\n", n_products);
    print_reactions(&c);

    compute_reactions(&c);

    return 0;
}

