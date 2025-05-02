#include <assert.h>
#include <mpi.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "solver.h"
#include "util.h"

static size_t n_vertices = 0;
static size_t n_edges = 0;
static char *filename = NULL;

void print_usage() {
  fprintf(stderr, "Usage: test_solver_distributed -n <n_vertices> -nnz <n_edges> -f <filename>\n");
  fprintf(stderr, "  -n <n_vertices>  Number of vertices in the graph\n");
  fprintf(stderr, "  -nnz <n_edges>       Number of non-zero elements in the graph\n");
  fprintf(stderr, "  -f <filename>    Output filename for the graph\n");
}

int parse_args(int argc, char *argv[], bool silent) {
  while (argc > 1) {
    if (strcmp(argv[1], "-n") == 0) {
      n_vertices = strtoul(argv[2], NULL, 10);
      argc -= 2;
      argv += 2;
    } else if (strcmp(argv[1], "-nnz") == 0) {
      n_edges = strtoul(argv[2], NULL, 10);
      argc -= 2;
      argv += 2;
    } else if (strcmp(argv[1], "-f") == 0) {
      filename = argv[2];
      argc -= 2;
      argv += 2;
    } else {
      if (!silent) {
        print_usage();
        fprintf(stderr, "Unknown argument: %s\n", argv[1]);
      }
      return 1;
    }
  }
  if (n_vertices == 0) {
    if (!silent) {
      print_usage();
      fprintf(stderr, "Number of vertices must be specified with -n\n");
    }
    return 1;
  }
  if (n_edges == 0) {
    if (!silent) {
      print_usage();
      fprintf(stderr, "Number of non-zero elements must be specified with -nnz\n");
    }
    return 1;
  }
  if (filename == NULL) {
    if (!silent) {
      print_usage();
      fprintf(stderr, "Output filename must be specified with -f\n");
    }
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  assert(parse_args(argc, argv, rank != 0) == 0);
  printf("[rank %02d] initialized; size: %d\n", rank, size);

  double t01_start = 0;
  double t02_create_random_matrix = 0;
  double t03_etc = 0;
  double t04_detect_subgraph = 0;
  double t05_color_cliquelike = 0;
  double t06_as_dot_color = 0;
  double t07_verify_coloring = 0;

  struct matrix *m;
  if (rank == 0) {
    t01_start = get_wtime();
    printf("matrix_create_random(%zu, %zu)\n", n_vertices, n_edges);
    m = matrix_create_random(n_vertices, n_edges);
    assert(m != NULL);
    assert(m->nnz == 2*n_edges);
    assert(m->n_vertices == n_vertices);
    t02_create_random_matrix = get_wtime();
  } else {
    m = malloc(sizeof(struct matrix));
    m->n_vertices = n_vertices;
    m->nnz = 2*n_edges;
    m->col_index = malloc(m->nnz * sizeof(number_t));
    m->row_index = malloc((n_vertices + 1) * sizeof(number_t));
  }
  if (rank == 0) {
    printf("broadcasting matrix\n");
  }
  int result = MPI_Bcast(m->col_index, m->nnz, NUMBER_T_MPI, 0, MPI_COMM_WORLD);
  assert(result == MPI_SUCCESS);
  MPI_Barrier(MPI_COMM_WORLD);
  result = MPI_Bcast(m->row_index, (m->n_vertices + 1), NUMBER_T_MPI, 0, MPI_COMM_WORLD);
  assert(result == MPI_SUCCESS);
  if (rank == 0) {
    printf("done broadcasting matrix\n");
  }

  {
    char *filename[100] = {0};
    sprintf(filename, "/tmp/matrix_%d.dot", rank);
    FILE *f = fopen(filename, "w");
    assert(f != NULL);
    matrix_as_dot(m, f);
    fclose(f);
  }
  
  if (rank == 0) {
    printf("allocate coloring - %lx bytes\n", sizeof(struct coloring));
  }
  struct coloring *c = malloc(sizeof(struct coloring));
  assert(c != NULL);
  if (rank == 0) {
    printf("allocate coloring.colors - %lx bytes\n", m->n_vertices * sizeof(number_t));
  }
  c->colors = calloc(m->n_vertices, sizeof(number_t));
  assert(c->colors != NULL);
  c->colors_size = m->n_vertices;

  if (rank == 0) {
    printf("allocate degree - %lx bytes\n", m->n_vertices * sizeof(size_t));
  }
  size_t *degree = malloc(m->n_vertices * sizeof(size_t));
  assert(degree != NULL);
  matrix_degree(m, degree);
  size_t max_degree = 0;
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (degree[i] > max_degree) {
      max_degree = degree[i];
    }
  }
  size_t k = max_degree + 1;

  if (rank == 0) {
    t03_etc = get_wtime();
    printf("max degree: %zu\n", max_degree);
    printf("k: %zu\n", k);
  }
  size_t subgraphs_length;
  struct subgraph *subgraphs;
  if (rank == 0) {
    subgraphs = detect_subgraph(m, k, &subgraphs_length);
    printf("there are %zu subgraphs\n", subgraphs_length);
  }
  result = MPI_Bcast(&subgraphs_length, sizeof(size_t), MPI_BYTE, 0, MPI_COMM_WORLD);
  assert(result == MPI_SUCCESS);

  size_t subgraphs_length_for_me = subgraphs_length / size;
  if ((size_t) rank < subgraphs_length % size) {
    subgraphs_length_for_me++;
  }
  struct subgraph *my_subgraphs = malloc(sizeof(struct subgraph) * subgraphs_length_for_me);
  if (rank == 0) {
    for (size_t i = 0; i < subgraphs_length; i++) {
      int dest_rank = i % size;
      if (dest_rank == rank) {
        my_subgraphs[i / size] = subgraphs[i];
      } else {
        // probably we should MPI_Type_create_struct here but I am too lazy
        printf("[rank %02d] sending subgraph (root index %zu) to rank %d\n", rank, i, dest_rank);
        int result = MPI_Send(subgraphs[i].vertices, m->n_vertices * sizeof(bool), MPI_BYTE, dest_rank, 0, MPI_COMM_WORLD);
        assert(result == MPI_SUCCESS);
      }
    }
    t04_detect_subgraph = get_wtime();
  } else {
    printf("[rank %02d] receiving %zu subgraphs\n", rank, subgraphs_length_for_me);
    for (size_t i = 0; i < subgraphs_length_for_me; i++) {
      printf("[rank %02d] receiving subgraph (my index %zu)\n", rank, i);
      my_subgraphs[i].vertices = malloc(m->n_vertices * sizeof(bool));
      assert(my_subgraphs[i].vertices != NULL);
      int result = MPI_Recv(my_subgraphs[i].vertices, m->n_vertices * sizeof(bool), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      assert(result == MPI_SUCCESS);
    }
    printf("[rank %02d] received %zu subgraphs\n", rank, subgraphs_length_for_me);
  }

  for (size_t i = 0; i < subgraphs_length_for_me; i++) {
    struct subgraph s = my_subgraphs[i];
    size_t vertex_count = 0;
    for (size_t j = 0; j < m->n_vertices; j++) {
      if (s.vertices[j]) {
        vertex_count++;
      }
    }
    printf("[rank %02d] subgraph %zu has %zu vertices\n", rank, i, vertex_count);
    //struct matrix *selected = matrix_select(m, s.vertices);
    //color_cliquelike(selected, c, k);
    color_cliquelike(m, c, k, s.vertices);
  }
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (degree[i] == 0) {
      c->colors[i] = 1;
    }
  }
  if (rank == 0) {
    for (size_t i = 0; i < (size_t) size-1; i++) {
      MPI_Status status;
      struct coloring *c_subgraph = malloc(sizeof(struct coloring));
      c_subgraph->colors = malloc(m->n_vertices * sizeof(number_t));
      result = MPI_Recv(c_subgraph->colors, m->n_vertices * sizeof(number_t), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      assert(result == MPI_SUCCESS);
      int source_rank = status.MPI_SOURCE;
      printf("received coloring from rank %d\n", source_rank);
      // get the vertices this rank was assigned
      for (size_t i = 0; i < subgraphs_length; i++) {
        int dest_rank = i % size;
        if (dest_rank != source_rank) {
          continue;
        }
        for (size_t j = 0; j < m->n_vertices; j++) {
          if (subgraphs[i].vertices[j]) {
            c->colors[j] = c_subgraph->colors[j];
          }
        }
      }
    }
    printf("coloring done\n");
    t05_color_cliquelike = get_wtime();
    color_cliquelike(m, c, k, NULL);
  } else {
    printf("[rank %02d] sending coloring to rank 0\n", rank);
    result = MPI_Send(c->colors, m->n_vertices * sizeof(number_t), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    assert(result == MPI_SUCCESS);
  }

  if (rank == 0) {
    printf("opening file %s\n", filename);
    FILE *f = fopen(filename, "w");
    assert(f != NULL);
    matrix_as_dot_color(m, f, c);
    fclose(f);
    t06_as_dot_color = get_wtime();

    if (!matrix_verify_coloring(m, c, false)) {
      fprintf(stderr, "Coloring verification failed\n");
      assert(false);
    }
    t07_verify_coloring = get_wtime();

    printf("=== timing report ===\n");
    printf("matrix_create_random:   %03f s\n", t02_create_random_matrix - t01_start);
    printf("matrix_degree:          %03f s\n", t03_etc - t02_create_random_matrix);
    printf("detect_subgraph:        %03f s\n", t04_detect_subgraph - t03_etc);
    printf("color_cliquelike:       %03f s\n", t05_color_cliquelike - t04_detect_subgraph);
    printf("matrix_as_dot_color:    %03f s\n", t06_as_dot_color - t05_color_cliquelike);
    printf("matrix_verify_coloring: %03f s\n", t07_verify_coloring - t06_as_dot_color);
    printf("=== end timing report ===\n");
    printf("number of OMP threads:  %d\n", get_num_omp_threads());
  }

  printf("[rank %02d] done, waiting for all ranks\n", rank);
  MPI_Barrier(MPI_COMM_WORLD);
  printf("[rank %02d] exiting\n", rank);

  MPI_Finalize();
  return 0;
}

