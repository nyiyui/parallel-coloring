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

  struct coloring *c = malloc(sizeof(struct coloring));
  if (c == NULL) {
    fclose(f);
    matrix_destroy(m, free);
    return 1;
  }
  c->colors = calloc(m->n_vertices, sizeof(number_t));
  if (c->colors == NULL) {
    free(c);
    fclose(f);
    matrix_destroy(m, free);
    return 1;
  }
  c->colors_size = m->n_vertices;
  
  /*matrix_as_dot(m, f);*/
  matrix_as_dot_color(m, f, c);
  
  fclose(f);
  
  matrix_destroy(m, free);
  
  return 0;
}
