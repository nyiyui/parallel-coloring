#include <stdlib.h>

#include "graph.h"
#include "solver.h"

int main(int argc, char *argv[]) {
  char *filename = argv[1];
  if (argc != 2) {
    filename = "test_graph.dot";
  }
  struct matrix *m = matrix_create_random(0x40, 0x20);
  if (m == NULL) {
    return 1;
  }

  matrix_print(m);
  
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    matrix_destroy(m);
    return 1;
  }

  struct coloring *c = malloc(sizeof(struct coloring));
  if (c == NULL) {
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  c->colors = calloc(m->n_vertices, sizeof(number_t));
  if (c->colors == NULL) {
    free(c);
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  c->colors_size = m->n_vertices;

  printf("luby_maximal_independent_set\n");
  luby_maximal_independent_set(m, c, 1, NULL);
  printf("luby_maximal_independent_set done\n");
  matrix_verify_coloring(m, c, true);
  printf("matrix_verify_coloring done\n");
  matrix_as_dot_color(m, f, c);
  printf("matrix_as_dot_color done\n");
  
  fclose(f);
  matrix_destroy(m);
  return 0;
}
