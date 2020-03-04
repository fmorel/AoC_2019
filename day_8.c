#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    char *data;
    int n_digits[3];
} Layer;

typedef struct {
    int n_layers;
    int width;
    int height;
    Layer *layers;
    Layer render;
} Image;

static int parse_image(char *filename, Image *img)
{
    int i, size;
    FILE *f = fopen(filename, "r");
    char *layer_data;
    Layer *l;
    
    if (!f) {
        printf("Could not open file\n");
        exit(-1);
    }

    size = img->width * img->height;
    layer_data = malloc(size+1);

    img->layers = NULL;
    img->n_layers = 0;
    while (fgets(layer_data, size+1, f)) {
        if (strlen(layer_data) != size)
            break;
        img->layers = realloc(img->layers,  (img->n_layers+1) * sizeof(Layer));
        l = &img->layers[img->n_layers];
        l->data = malloc(size);
        memset(l->n_digits, 0, sizeof(l->n_digits));
        for (i = 0; i < size; i++) {
            l->data[i] = layer_data[i] - '0';
            if(!(l->data[i] >= 0 && l->data[i] <= 2)) {
                printf("Failure : l=%d, i=%d : %c, %d\n", img->n_layers, i, layer_data[i], l->data[i]);
                return 0;
            }
            l->n_digits[l->data[i]]++;
        }
        img->n_layers++;
    }

    printf("%d layers parsed\n", img->n_layers);
    return img->n_layers;
}

static int free_image(Image *img)
{
    int i;

    for (i = 0; i < img->n_layers; i++) {
        free(img->layers[i].data);
    }
    free(img->layers);
}

static void print_image(Image *img)
{
    int i, j, k;
    for (i = 0, k = 0; i < img->height; i++) {
        for (j = 0; j < img->width; j++, k++) {
            if (img->render.data[k])
                printf("|");
            else
                printf(" ");
        }
        printf("\n");
    }
}
    
static int fewest_zero(Image *img)
{
    int i, min_zero;
    int ret;

    min_zero = img->width * img->height; 
    for (i = 0; i < img->n_layers; i++) {
        if (img->layers[i].n_digits[0] < min_zero) {
            min_zero = img->layers[i].n_digits[0];
            ret = img->layers[i].n_digits[1] * img->layers[i].n_digits[2];
        }
    }

    return ret;
}


static void render_image(Image *img)
{
    int i, j, size;
    Layer *l;
    
    size = img->width * img->height;
    img->render.data = malloc(size);
    memset(img->render.data, 2, size);

    for (i = 0; i < img->n_layers; i++) {
        l = &img->layers[i];
        for (j = 0; j < size; j++) {
            if (img->render.data[j] == 2)
                img->render.data[j] = l->data[j];
        }
    }
}


int main(int argc, char **argv)
{
    Image img;
    
    img.width = 25;
    img.height = 6;

    if (!parse_image(argv[1], &img))
        goto cleanup;

    printf("Day 1 %d\n", fewest_zero(&img));

    render_image(&img);
    print_image(&img);

cleanup:
    free_image(&img);
    return 0;
}
