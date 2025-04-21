#include <stdlib.h>

#include "graph.h"

int main(void) {
  struct matrix_al *m = matrix_al_create(10, malloc);
  if (m == NULL) {
    return 1;
  }

  matrix_al_fill_random(m, 20);

  matrix_al_print(m);
  
  FILE *f = fopen("graph.dot", "w");
  if (f == NULL) {
    matrix_al_destroy(m, free);
    return 1;
  }
  
  matrix_al_as_dot(m, f);
  
  fclose(f);
  
  matrix_al_destroy(m, free);
  
  return 0;
}
