#include <stdlib.h>

#include "graph.h"

int main(void) {
  struct matrix_csr *m = matrix_csr_create(10, 10, malloc);
  if (m == NULL) {
    return 1;
  }

  matrix_csr_fill_random(m);

  matrix_csr_print(m);
  
  FILE *f = fopen("graph.dot", "w");
  if (f == NULL) {
    matrix_csr_destroy(m, free);
    return 1;
  }
  
  matrix_csr_as_dot(m, f);
  
  fclose(f);
  
  matrix_csr_destroy(m, free);
  
  return 0;
}
