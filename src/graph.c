#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "graph.h"

char *color_names[] = {
  "black",
  "red",
  "blue",
  "purple",
  "pink",
  "brown",
  "gray"
};

const size_t color_names_length = sizeof(color_names) / sizeof(char *);

struct matrix_al *matrix_al_create(size_t pairs_size, number_t n_vertices, void *malloc(size_t)) {
  struct matrix_al *m = malloc(sizeof(struct matrix_al));
  if (m == NULL) {
    return NULL;
  }
  m->pairs = malloc(pairs_size * sizeof(struct matrix_al_pair));
  if (m->pairs == NULL) {
    free(m);
    return NULL;
  }
  m->pairs_size = pairs_size;
  m->n_vertices = n_vertices;
  return m;
}

void matrix_al_destroy(struct matrix_al *m, void free(void *)) {
  if (m == NULL) {
    return;
  }
  if (m->pairs != NULL) {
    free(m->pairs);
  }
  free(m);
}

void matrix_al_print(struct matrix_al *m) {
  if (m == NULL) {
    return;
  }
  for (size_t i = 0; i < m->pairs_size; i++) {
    printf("(%lu, %lu)\n", m->pairs[i].i, m->pairs[i].j);
  }
}

void matrix_al_as_dot(struct matrix_al *m, FILE *f) {
  if (m == NULL || f == NULL) {
    return;
  }
  fprintf(f, "graph G {\n");
  for (size_t i = 0; i < m->pairs_size; i++) {
    if (m->pairs[i].i >= m->pairs[i].j) {
      continue;
    }
    fprintf(f, "  %lu -- %lu;\n", m->pairs[i].i, m->pairs[i].j);
  }
  fprintf(f, "}\n");
}

void matrix_as_dot_color(const struct matrix *m, FILE *f, const struct coloring *c) {
  if (m == NULL || f == NULL || c == NULL) {
    return;
  }
  fprintf(f, "graph G {\n");
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (c->colors[i] < color_names_length) {
      fprintf(f, "  %lu [color=%s];\n", i, color_names[c->colors[i]]);
    }
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      fprintf(f, "  %lu -- %lu;\n", i, m->col_index[j]);
    }
  }
  fprintf(f, "}\n");
}

bool matrix_al_query(struct matrix_al *m, number_t i, number_t j) {
  if (m == NULL) {
    return false;
  }
  // TODO: binary search
  for (size_t k = 0; k < m->pairs_size; k++) {
    if (m->pairs[k].i == i && m->pairs[k].j == j) {
      return true;
    }
  }
  return false;
}

void matrix_al_fill_random(struct matrix_al *m) {
  if (m == NULL) {
    return;
  }
  for (size_t count = 0; count < m->pairs_size;) {
    number_t i = random() % m->n_vertices;
    number_t j = random() % m->n_vertices;
    if (i > j) {
      number_t tmp = i;
      i = j;
      j = tmp;
    }
    if (matrix_al_query(m, i, j)) {
      continue;
    }
    m->pairs[count].i = i;
    m->pairs[count].j = j;
    count++;
  }
}

bool matrix_al_verify_coloring(struct matrix_al *m, struct coloring *c) {
  if (m == NULL || c == NULL) {
    return false;
  }
  for (size_t i = 0; i < m->pairs_size; i++) {
    if (c->colors[m->pairs[i].i] == c->colors[m->pairs[i].j]) {
      printf("Invalid coloring at (%lu, %lu)\n", m->pairs[i].i, m->pairs[i].j);
      return false;
    }
  }
  return true;
}

struct matrix *matrix_create(const size_t n_vertices, const size_t nnz) {
#ifdef DEBUG
  printf("allocate matrix - %lx bytes\n", sizeof(struct matrix));
#endif
  struct matrix *m = malloc(sizeof(struct matrix));
  if (m == NULL) {
    return NULL;
  }
  m->n_vertices = n_vertices;
  m->nnz = nnz;
#ifdef DEBUG
  printf("allocate matrix.col_index - %lx bytes\n", nnz * sizeof(number_t));
#endif
  m->col_index = malloc(nnz * sizeof(number_t));
  if (m->col_index == NULL) {
    free(m);
    return NULL;
  }
#ifdef DEBUG
  printf("allocate matrix.row_index - %lx bytes\n", (n_vertices + 1) * sizeof(number_t));
#endif
  m->row_index = malloc((n_vertices + 1) * sizeof(number_t));
  if (m->row_index == NULL) {
    free(m->col_index);
    free(m);
    return NULL;
  }
  return m;
}

struct matrix *matrix_create_random(const size_t n_vertices, const size_t n_edges) {
  // TODO: make a random adjacency matrix first, then convert it to CSR
  //       inefficient but easy to implement
#ifdef DEBUG
  printf("allocate matrix - %lx bytes\n", (n_vertices * n_vertices) * sizeof(number_t));
#endif
  number_t *am = calloc(n_vertices * n_vertices, sizeof(number_t));
  if (am == NULL) {
    return NULL;
  }
  for (size_t count = 0; count < n_edges;) {
    number_t i = random() % n_vertices;
    number_t j = random() % n_vertices;
    if (am[i * n_vertices + j] ||
        am[j * n_vertices + i] ||
        i == j) {
      continue;
    }
    am[i * n_vertices + j] = 1;
    am[j * n_vertices + i] = 1;
    count++;
  }

  struct matrix *m = matrix_create(n_vertices, 2*n_edges);
  if (m == NULL) {
    free(am);
    return NULL;
  }
  // fill row_index
  {
    size_t total_nz = 0;
    for (size_t i = 0; i < m->n_vertices; i++) {
      m->row_index[i] = total_nz;
      for (size_t j = 0; j < m->n_vertices; j++) {
        if (am[i * m->n_vertices + j]) {
          total_nz++;
        }
      }
    }
    m->row_index[m->n_vertices] = total_nz;
  }
  // fill col_index
  {
    size_t total_nz = 0;
    for (size_t i = 0; i < m->n_vertices; i++) {
      for (size_t j = 0; j < m->n_vertices; j++) {
        if (am[i * m->n_vertices + j]) {
          m->col_index[total_nz] = j;
          total_nz++;
        }
      }
    }
  }
  free(am);
  // verify matrix
  for (size_t i = 0; i < m->nnz; i++) {
    assert(m->col_index[i] < m->n_vertices);
  }
  for (size_t i = 0; i < m->n_vertices; i++) {
    assert(m->row_index[i] < m->nnz);
  }
  assert(m->row_index[m->n_vertices] == m->nnz);
  return m;
}

void matrix_destroy(struct matrix *m) {
  if (m == NULL) {
    return;
  }
  if (m->col_index != NULL) {
    free(m->col_index);
  }
  if (m->row_index != NULL) {
    free(m->row_index);
  }
  free(m);
}

void matrix_print(const struct matrix *m) {
  if (m == NULL) {
    return;
  }
  // print like a normal matrix
  for (size_t i = 0; i < m->n_vertices; i++) {
    printf("% 2ld: ", i);
    for (size_t j = 0; j < m->n_vertices; j++) {
      bool found = false;
      for (size_t k = m->row_index[i]; k < m->row_index[i + 1]; k++) {
        if (m->col_index[k] == j) {
          found = true;
          break;
        }
      }
      printf(found ? "1 " : "_ ");
    }
    printf("\n");
  }
}

void matrix_as_dot(const struct matrix *m, FILE *f) {
  if (m == NULL || f == NULL) {
    return;
  }
  fprintf(f, "graph G {\n");
  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      fprintf(f, "  %lu -- %lu;\n", i, m->col_index[j]);
    }
  }
  fprintf(f, "}\n");
}

bool matrix_query(const struct matrix *m, number_t i, number_t j) {
  if (m == NULL) {
    return false;
  }
  if (i > j) {
    number_t tmp = i;
    i = j;
    j = tmp;
  }
  for (size_t k = m->row_index[i]; k < m->row_index[i + 1]; k++) {
    if (m->col_index[k] == j) {
      return true;
    }
  }
  return false;
}

bool matrix_verify_coloring(const struct matrix *m, const struct coloring *c, const bool ignore_zero) {
  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      if (c->colors[i] == c->colors[m->col_index[j]] && (!ignore_zero || c->colors[i] != 0)) {
        if (c->colors[i] == 0) {
          printf("Uncolored vertex %lu\n", i);
        } else {
          printf("Invalid coloring at (%lu, %lu)\n", i, m->col_index[j]);
        }
        return false;
      }
    }
  }
  return true;
}

struct matrix *matrix_induce(const struct matrix *m, const bool *take, number_t *new_vertex_out) {
  if (m == NULL || take == NULL) {
    return NULL;
  }

  // === prepare n_vertices and nnz of induced matrix ===
  size_t induced_n_vertices = 0;
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (take[i]) {
      new_vertex_out[i] = induced_n_vertices++;
    } else {
      new_vertex_out[i] = -1;
    }
  }
  // to get nnz, count edges that have both ends in take
  size_t induced_nnz = 0;
  number_t *edge_count_per_row = calloc(m->n_vertices, sizeof(number_t));
  if (edge_count_per_row == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = m->row_index[i]; j < m->row_index[i+1]; j++) {
      if (take[i] && take[m->col_index[j]]) {
        induced_nnz++;
        edge_count_per_row[i]++;
      }
    }
  }

  // === create and fill induced matrix ===
  struct matrix *induced = matrix_create(induced_n_vertices, induced_nnz);
  if (induced == NULL) {
    free(edge_count_per_row);
    return NULL;
  }

  // fill row_index
  {
    size_t total_nz = 0;
    for (size_t i = 0, j = 0; i < m->n_vertices; i++) {
      if (take[i]) {
        induced->row_index[j] = total_nz;
        total_nz += edge_count_per_row[i];
        j++;
      }
    }
  }

  // fill col_index
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (take[i]) {
      for (size_t j = m->row_index[i]; j < m->row_index[i+1]; j++) {
        number_t old_destination = m->col_index[j];
        if (!take[old_destination]) {
          continue;
        }
        number_t new_destination = new_vertex_out[old_destination];
        number_t new_source = new_vertex_out[i];
        if (new_source >= new_destination) {
          number_t tmp = new_source;
          new_source = new_destination;
          new_destination = tmp;
        }
        assert(edge_count_per_row[i] > 0);
        induced->col_index[induced->row_index[new_source] + edge_count_per_row[i] - 1] = new_destination;
        edge_count_per_row[i]--;
      }
    }
  }
  free(edge_count_per_row);
  return induced;
}

void matrix_iterate_edges(const struct matrix *m, const void (*f)(number_t, number_t, void *), void *data) {
  for (size_t i = 0; i < m->n_vertices; i++) {
    // _OPENMP: inner loop is serial, but inner loop has maximum of max(degree) iterations,
    //          which is expected to be small (<10)
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      f(i, m->col_index[j], data);
    }
  }
}

void matrix_degree(const struct matrix *m, size_t *degree) {
  assert(m != NULL);
  assert(degree != NULL);
  for (size_t i = 0; i < m->n_vertices; i++) {
    degree[i] = m->row_index[i + 1] - m->row_index[i];
  }
}

struct matrix *matrix_select(const struct matrix *m, const bool *select) {
  assert(m != NULL);
  assert(select != NULL);

  size_t selected_nnz = 0;
  // iterate all edges and count selected edges (i.e. have selected vertices on both ends)

  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
      size_t u = i;
      size_t v = m->col_index[j];
      if (select[u] && select[v] && matrix_query(m, u, v)) {
        selected_nnz++;
      }
    }
  }

  struct matrix *m2 = matrix_create(m->n_vertices, selected_nnz);
  if (m2 == NULL) {
    return NULL;
  }

  {
    size_t total_nz = 0;
    for (size_t i = 0; i < m2->n_vertices; i++) {
      m2->row_index[i] = total_nz;
      for (size_t j = 0; j < m->n_vertices; j++) {
        if (select[i] && select[j] && matrix_query(m, i, j)) {
          m2->col_index[total_nz] = j;
          total_nz++;
        }
      }
    }
    assert(total_nz == selected_nnz);
  }

  return m2;
}

void matrix_as_dot_subgraph_color(const struct matrix *m, FILE *f, const struct subgraph *subgraphs, const size_t subgraphs_length, const struct coloring *c) {
  if (m == NULL || f == NULL || subgraphs == NULL || subgraphs_length <= 0 || c == NULL) {
    return;
  }
  fprintf(f, "graph G {\n");
  for (size_t subgraph_index = 0; subgraph_index < subgraphs_length; subgraph_index++) {
    fprintf(f, "  subgraph cluster_%zu {\n", subgraph_index);
    for (size_t i = 0; i < m->n_vertices; i++) {
      if (!subgraphs[subgraph_index].vertices[i]) {
        continue;
      }
      if (c->colors[i] < color_names_length) {
        fprintf(f, "    %lu [color=%s];\n", i, color_names[c->colors[i]]);
      }
      for (size_t j = m->row_index[i]; j < m->row_index[i + 1]; j++) {
        if (i >= m->col_index[j]) {
          continue;
        }
        fprintf(f, "    %lu -- %lu;\n", i, m->col_index[j]);
      }
    }
    fprintf(f, "  }\n");
  }
  fprintf(f, "}\n");
}
