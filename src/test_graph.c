#include <stdlib.h>
#include <assert.h>

#include "graph.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <dot_path>\n", argv[0]);
    return 1;
  }
  struct matrix *m = matrix_create_random(0x40, 0x20);
  if (m == NULL) {
    return 1;
  }

  matrix_print(m);
  
  FILE *f = fopen(argv[1], "w");
  if (f == NULL) {
    perror("fopen");
    assert(0);
  }

  struct coloring *c = malloc(sizeof(struct coloring));
  assert(c != NULL);
  c->colors = calloc(m->n_vertices, sizeof(number_t));
  assert(c->colors != NULL);
  c->colors_size = m->n_vertices;
  
  /*matrix_as_dot(m, f);*/
  matrix_as_dot_color(m, f, c);
  fclose(f);

  bool *select = calloc(m->n_vertices, sizeof(bool));
  assert(select != NULL);
  for (size_t i = 0; i < m->n_vertices; i++) {
    select[i] = (random() % 2) == 0;
  }
  struct matrix *m2 = matrix_select(m, select);

  // verify matrix_select
  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = 0; j < m->n_vertices; j++) {
      if (select[i] && select[j]) {
        if (matrix_query(m, i, j) != matrix_query(m2, i, j)) {
          printf("matrix_select failed at (%lu, %lu)\n", i, j);
          assert(0);
        }
      }
    }
  }

  matrix_destroy(m);
  matrix_destroy(m2);

  free(select);
  free(c->colors);
  free(c);
  
  return 0;
}
