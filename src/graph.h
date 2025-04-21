#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define number_t uint32_t
#define number_bits (sizeof(number_t) * 8)

// Note that this matrix only has (i, j) where i < j
struct matrix_csr {
  number_t *col_index;
  size_t col_index_size;
  number_t *row_index;
  size_t row_index_size;
};

struct matrix_csr *matrix_csr_create(size_t col_index_size,
                                     size_t row_index_size,
                                     void *malloc(size_t));

void matrix_csr_destroy(struct matrix_csr *m, void free(void *));

void matrix_csr_print(struct matrix_csr *m);

void matrix_csr_as_dot(struct matrix_csr *m, FILE *f);

bool matrix_csr_query(struct matrix_csr *m, number_t i, number_t j);

void matrix_csr_fill_random(struct matrix_csr *m);
