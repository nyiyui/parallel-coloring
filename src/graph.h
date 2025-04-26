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
  bool *v; // v[i] iff i is in the graph, length of v is n_vertices
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

struct matrix {
  size_t n_vertices;
  size_t nnz;
  number_t *col_index;  // nnz elements
  number_t *row_index;  // n_vertices + 1 elements
};

struct matrix *matrix_create(size_t n_vertices, size_t nnz, void *malloc(size_t));

struct matrix *matrix_create_random(size_t n_vertices, size_t nnz, void *malloc(size_t));

void matrix_ensure_symmetric(struct matrix *m);

void matrix_destroy(struct matrix *m, void free(void *));

void matrix_print(struct matrix *m);

void matrix_as_dot(struct matrix *m, FILE *f);

bool matrix_query(struct matrix *m, number_t i, number_t j);

bool matrix_verify_coloring(struct matrix *m, struct coloring *c);

struct matrix *matrix_induce(struct matrix *m, bool *take, number_t *new_vertex_out, void *malloc(size_t));
