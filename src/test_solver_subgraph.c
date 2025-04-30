#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "util.h"
#include "solver.h"

static size_t n_vertices = 0;
static size_t nnz = 0;
static char *filename = NULL;
static char *filename2 = NULL;

void print_usage() {
  fprintf(stderr, "Usage: test_solver_subgraph -n <n_vertices> -nnz <nnz> -f <filename>\n");
  fprintf(stderr, "  -n <n_vertices>   Number of vertices in the graph\n");
  fprintf(stderr, "  -nnz <nnz>       Number of non-zero elements in the graph\n");
  fprintf(stderr, "  -f <filename>    Output filename for the graph\n");
  fprintf(stderr, "  -f2 <filename>    Output filename for the 2nd graph\n");
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
    } else if (strcmp(argv[1], "-f2") == 0) {
      filename2 = argv[2];
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
  if (filename2 == NULL) {
    print_usage();
    fprintf(stderr, "Output filename for the 2nd graph must be specified with -f2\n");
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

  struct coloring c = { 
    .colors = calloc(m->n_vertices, sizeof(number_t)),
    .colors_size = m->n_vertices
  };
  assert(c.colors != NULL);
  
  printf("opening file2 %s\n", filename2);
  FILE *f2 = fopen(filename2, "w");
  assert(f2 != NULL);
  matrix_as_dot_color(m, f2, &c);
  fclose(f2);
  printf("opening file %s\n", filename);
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    matrix_destroy(m);
    return 1;
  }

  printf("allocate degree - %lx bytes\n", m->n_vertices * sizeof(size_t));
  size_t *degree = malloc(m->n_vertices * sizeof(size_t));
  assert(degree != NULL);
  matrix_degree(m, degree);
  size_t max_degree = 0;
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (degree[i] > max_degree) {
      max_degree = degree[i];
    }
  }
  printf("max degree: %zu\n", max_degree);

  double t03_etc = get_wtime();

  size_t subgraphs_length;
  struct subgraph *s = detect_subgraph(m, max_degree+1, &subgraphs_length);
  if (s == NULL) {
    free(degree);
    fclose(f);
    matrix_destroy(m);
    return 1;
  }
  printf("subgraphs_length: %zu\n", subgraphs_length);

  double t04_detect_subgraph = get_wtime();

  for (size_t i = 0; i < subgraphs_length; i++) {
    size_t count = 0;
    for (size_t j = 0; j < m->n_vertices; j++) {
      if (s[i].vertices[j]) {
        count++;
      }
    }
    printf("subgraph %zu has %zu vertices\n", i, count);
  }
  matrix_as_dot_subgraph_color(m, f, s, subgraphs_length, &c);

  double t05_dot = get_wtime();

  printf("=== timing report ===\n");
  printf("matrix_create_random:   %03f s\n", t02_create_random_matrix - t01_start);
  printf("matrix_degree:          %03f s\n", t03_etc - t02_create_random_matrix);
  printf("detect_subgraph:        %03f s\n", t04_detect_subgraph - t03_etc);
  printf("matrix_as_dot_subgraph: %03f s\n", t05_dot - t04_detect_subgraph);
  printf("=== end timing report ===\n");
  printf("number of OMP threads:  %d\n", get_num_omp_threads());

  for (size_t i = 0; i < subgraphs_length; i++) {
    free(s[i].vertices);
  }
  free(s);
  fclose(f);
  matrix_destroy(m);
  free(degree);
  return 0;
}
