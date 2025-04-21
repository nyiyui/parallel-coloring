#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define number_t uint32_t
#define number_bits (sizeof(number_t) * 8)

// Note that this pair only has (i, j) where i < j
struct matrix_al_pair {
  number_t i;
  number_t j;
};

struct matrix_al {
  // pairs are first sorted by i then j
  struct matrix_al_pair *pairs;
  size_t pairs_size;
  number_t n_vertices;
};

struct matrix_al *matrix_al_create(size_t pairs_size, number_t n_vertices, void *malloc(size_t));

void matrix_al_destroy(struct matrix_al *m, void free(void *));

void matrix_al_print(struct matrix_al *m);

void matrix_al_as_dot(struct matrix_al *m, FILE *f);

bool matrix_al_query(struct matrix_al *m, number_t i, number_t j);

void matrix_al_fill_random(struct matrix_al *m);

struct coloring {
  number_t *colors;
  size_t colors_size;
};

bool matrix_al_verify_coloring(struct matrix_al *m, struct coloring *c);
