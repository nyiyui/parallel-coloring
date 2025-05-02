#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "mpi.h"

#define number_t uint64_t
#define NUMBER_T_MPI MPI_UINT64_T
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
  number_t *colors; // note that zero is not a color, but a marker for uncolored
  size_t colors_size;
};

bool matrix_al_verify_coloring(struct matrix_al *m, struct coloring *c);

struct matrix {
  size_t n_vertices;
  size_t nnz;
  number_t *col_index;  // nnz elements
  number_t *row_index;  // n_vertices + 1 elements
};

struct matrix *matrix_create(const size_t n_vertices, const size_t nnz);

struct matrix *matrix_create_random(const size_t n_vertices, const size_t nnz);

void matrix_destroy(struct matrix *m);

void matrix_print(const struct matrix *m);

void matrix_as_dot(const struct matrix *m, FILE *f);

void matrix_as_dot_color(const struct matrix *m, FILE *f, const struct coloring *c);

bool matrix_query(const struct matrix *m, const number_t i, const number_t j);

bool matrix_verify_coloring(const struct matrix *m, const struct coloring *c, const bool ignore_zero);

struct matrix *matrix_induce(const struct matrix *m, const bool *take, number_t *new_vertex_out);

void matrix_iterate_edges(const struct matrix *m, const void (*f)(number_t, number_t, void *), void *data);

void matrix_degree(const struct matrix *m, size_t *degree);

struct matrix *matrix_select(const struct matrix *m, const bool *select);

struct subgraph {
    bool *vertices;
};

void matrix_as_dot_subgraph_color(const struct matrix *m, FILE *f, const struct subgraph *subgraphs, const size_t subgraphs_length, const struct coloring *c);
