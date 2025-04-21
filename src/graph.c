#include <stdlib.h>

#include "graph.h"

struct matrix_al *matrix_al_create(size_t pairs_size, number_t n_vertices, void *malloc(size_t)) {
  struct matrix_al *m = malloc(sizeof(struct matrix_al));
  if (m == NULL) {
    return NULL;
  }
  m->pairs = malloc(pairs_size * sizeof(struct matrix_al_pair));
  if (m->pairs == NULL) {
    free(m);
    return NULL;
  }
  m->pairs_size = pairs_size;
  m->n_vertices = n_vertices;
  return m;
}

void matrix_al_destroy(struct matrix_al *m, void free(void *)) {
  if (m == NULL) {
    return;
  }
  if (m->pairs != NULL) {
    free(m->pairs);
  }
  free(m);
}

void matrix_al_print(struct matrix_al *m) {
  if (m == NULL) {
    return;
  }
  for (size_t i = 0; i < m->pairs_size; i++) {
    printf("(%u, %u)\n", m->pairs[i].i, m->pairs[i].j);
  }
}

void matrix_al_as_dot(struct matrix_al *m, FILE *f) {
  if (m == NULL || f == NULL) {
    return;
  }
  fprintf(f, "graph G {\n");
  for (size_t i = 0; i < m->pairs_size; i++) {
    fprintf(f, "  %u -- %u;\n", m->pairs[i].i, m->pairs[i].j);
  }
  fprintf(f, "}\n");
}

bool matrix_al_query(struct matrix_al *m, number_t i, number_t j) {
  if (m == NULL) {
    return false;
  }
  // TODO: binary search
  for (size_t k = 0; k < m->pairs_size; k++) {
    if (m->pairs[k].i == i && m->pairs[k].j == j) {
      return true;
    }
  }
  return false;
}

void matrix_al_fill_random(struct matrix_al *m) {
  if (m == NULL) {
    return;
  }
  for (size_t count = 0; count < m->pairs_size;) {
    number_t i = random() % m->n_vertices;
    number_t j = random() % m->n_vertices;
    if (i > j) {
      number_t tmp = i;
      i = j;
      j = tmp;
    }
    if (matrix_al_query(m, i, j)) {
      continue;
    }
    m->pairs[count].i = i;
    m->pairs[count].j = j;
    count++;
  }
}

bool matrix_al_verify_coloring(struct matrix_al *m, struct coloring *c) {
  if (m == NULL || c == NULL) {
    return false;
  }
  for (size_t i = 0; i < m->pairs_size; i++) {
    if (c->colors[m->pairs[i].i] == c->colors[m->pairs[i].j]) {
      printf("Invalid coloring at (%u, %u)\n", m->pairs[i].i, m->pairs[i].j);
      return false;
    }
  }
  return true;
}
