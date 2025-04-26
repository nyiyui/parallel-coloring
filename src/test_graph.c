#include <stdlib.h>

#include "graph.h"

int main(void) {
  struct matrix *m = matrix_create_random(0x40, 0x20, malloc);
  if (m == NULL) {
    return 1;
  }

  matrix_print(m);
  
  FILE *f = fopen("graph.dot", "w");
  if (f == NULL) {
    matrix_destroy(m, free);
    return 1;
  }
  
  matrix_as_dot(m, f);
  
  fclose(f);
  
  matrix_destroy(m, free);
  
  return 0;
}
