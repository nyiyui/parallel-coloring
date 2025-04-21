#include <stdlib.h>

#include "graph.h"

struct matrix_csr *matrix_csr_create(size_t col_index_size,
                                     size_t row_index_size,
                                     void *malloc(size_t)) {
  struct matrix_csr *m = malloc(sizeof(struct matrix_csr));
  if (m == NULL) {
    return NULL;
  }
  m->col_index = malloc(col_index_size * sizeof(number_t));
  if (m->col_index == NULL) {
    free(m);
    return NULL;
  }
  m->row_index = malloc(row_index_size * sizeof(number_t));
  if (m->row_index == NULL) {
    free(m->col_index);
    free(m);
    return NULL;
  }
  m->col_index_size = col_index_size;
  m->row_index_size = row_index_size;
  return m;
}

void matrix_csr_destroy(struct matrix_csr *m, void free(void *)) {
  free(m->col_index);
  free(m->row_index);
  free(m);
}

void matrix_csr_print(struct matrix_csr *m) {
  printf("col_index: ");
  for (size_t i = 0; i < m->col_index_size; i++) {
    printf("%d ", m->col_index[i]);
  }
  printf("\nrow_index: ");
  for (size_t i = 0; i < m->row_index_size; i++) {
    printf("%d ", m->row_index[i]);
  }
  printf("\n");
}

void matrix_csr_as_dot(struct matrix_csr *m, FILE *f) {
  fprintf(f, "digraph G {\n");
  for (size_t i = 0; i < m->row_index_size - 1; i++) {
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      fprintf(f, "  %zu -> %zu ;\n", i, m->col_index[j]);
    }
  }
  fprintf(f, "}\n");
}

bool matrix_csr_query(struct matrix_csr *m, number_t i, number_t j) {
  if (i >= j) {
    number_t tmp = i;
    i = j;
    j = tmp;
  }
  if (i >= m->row_index_size || j >= m->col_index_size) {
    return false;
  }
  number_t row_start = m->row_index[i];
  number_t row_end = m->row_index[i + 1];
  for (number_t k = row_start; k < row_end; k++) {
    if (m->col_index[k] == j) {
      return true;
    }
  }
  return false;
}

void matrix_csr_fill_random(struct matrix_csr *m) {
  // given m->col_index_size entries, set each entry to a random value to make a random edge
  for (size_t i = 0; i < m->col_index_size; i++) {
    m->col_index[i] = random() % m->row_index_size;
  }
  for (size_t i = 0; i < m->row_index_size; i++) {
    bool increment = random() % 2;
    if (increment) {
      m->row_index[i] = m->row_index[i - 1] + 1;
    } else {
      m->row_index[i] = m->row_index[i - 1];
    }
  }
}
