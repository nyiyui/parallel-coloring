#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "graph.h"
#include "solver.h"
#include "util.h"

static size_t n_vertices = 0;
static size_t nnz = 0;
static char *filename = NULL;

void print_usage() {
  fprintf(stderr, "Usage: test_solver_color -n <n_vertices> -nnz <nnz> -f <filename>\n");
  fprintf(stderr, "  -n <n_vertices>   Number of vertices in the graph\n");
  fprintf(stderr, "  -nnz <nnz>       Number of non-zero elements in the graph\n");
  fprintf(stderr, "  -f <filename>    Output filename for the graph\n");
}

int parse_args(int argc, char *argv[]) {
  while (argc > 1) {
    if (strcmp(argv[1], "-n") == 0) {
      n_vertices = strtoul(argv[2], NULL, 10);
      argc -= 2;
      argv += 2;
    } else if (strcmp(argv[1], "-nnz") == 0) {
      nnz = strtoul(argv[2], NULL, 10);
      argc -= 2;
      argv += 2;
    } else if (strcmp(argv[1], "-f") == 0) {
      filename = argv[2];
      argc -= 2;
      argv += 2;
    } else {
      print_usage();
      fprintf(stderr, "Unknown argument: %s\n", argv[1]);
      return 1;
    }
  }
  if (n_vertices == 0) {
    print_usage();
    fprintf(stderr, "Number of vertices must be specified with -n\n");
    return 1;
  }
  if (nnz == 0) {
    print_usage();
    fprintf(stderr, "Number of non-zero elements must be specified with -nnz\n");
    return 1;
  }
  if (filename == NULL) {
    print_usage();
    fprintf(stderr, "Output filename must be specified with -f\n");
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (parse_args(argc, argv) != 0) {
    return 1;
  }

  double t01_start = get_wtime();
  printf("matrix_create_random(%zu, %zu)\n", n_vertices, nnz);
  struct matrix *m = matrix_create_random(n_vertices, nnz);
  if (m == NULL) {
    return 1;
  }
  double t02_create_random_matrix = get_wtime();

  // matrix_print(m);
  
  printf("opening file %s\n", filename);
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    matrix_destroy(m);
    return 1;
  }

  printf("allocate coloring - %x bytes\n", sizeof(struct coloring));
  struct coloring *c = malloc(sizeof(struct coloring));
  if (c == NULL) {
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  printf("allocate coloring.colors - %x bytes\n", m->n_vertices * sizeof(number_t));
  c->colors = calloc(m->n_vertices, sizeof(number_t));
  if (c->colors == NULL) {
    free(c);
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  c->colors_size = m->n_vertices;

  printf("allocate degree - %x bytes\n", m->n_vertices * sizeof(size_t));
  size_t *degree = malloc(m->n_vertices * sizeof(size_t));
  if (degree == NULL) {
    free(c->colors);
    free(c);
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  matrix_degree(m, degree);
  size_t max_degree = 0;
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (degree[i] > max_degree) {
      max_degree = degree[i];
    }
  }

  double t03_etc = get_wtime();

  printf("max degree: %zu\n", max_degree);

  color_cliquelike(m, c, max_degree);
  double t04_color_cliquelike = get_wtime();
  matrix_as_dot_color(m, f, c);
  double t05_as_dot_color = get_wtime();

  if (!matrix_verify_coloring(m, c, false)) {
    fprintf(stderr, "Coloring verification failed\n");
    free(c->colors);
    free(c);
    fclose(f);
    matrix_destroy(m);
    free(degree);
    return 1;
  }
  double t06_verify_coloring = get_wtime();

  printf("=== timing report ===\n");
  printf("matrix_create_random:   %03f s\n", t02_create_random_matrix - t01_start);
  printf("matrix_degree:          %03f s\n", t03_etc - t02_create_random_matrix);
  printf("color_cliquelike:       %03f s\n", t04_color_cliquelike - t03_etc);
  printf("matrix_as_dot_color:    %03f s\n", t05_as_dot_color - t04_color_cliquelike);
  printf("matrix_verify_coloring: %03f s\n", t06_verify_coloring - t05_as_dot_color);
  printf("=== end timing report ===\n");
  printf("number of OMP threads:  %d\n", get_num_omp_threads());
  
  fclose(f);
  matrix_destroy(m);
  free(c->colors);
  free(c);
  free(degree);
  return 0;
}
