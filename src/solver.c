#include "solver.h"

void union_assign(bool *a, bool *b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    a[i] = a[i] || b[i];
  }
}

void make_neighbourhood(struct matrix_al *m, bool *w, bool *n_w) {
  // 3.1 of Luby
  for (size_t i = 0; i < m->pairs_size; i++) {
    number_t u = m->pairs[i].i;
    number_t v = m->pairs[i].j;
    if (w[v]) {
      n_w[u] = true;
    }
    // handle the symmetric case
    if (w[u]) {
      n_w[v] = true;
    }
  }
}

void remove_edges_with(struct matrix_al *m, bool *w) {
  // 3.1 of Luby
  size_t new_size = 0;
  for (size_t i = 0; i < m->pairs_size; i++) {
    number_t u = m->pairs[i].i;
    number_t v = m->pairs[i].j;
    if (!(w[u] || w[v])) {
      // new_size is always less than or equal to i
      m->pairs[new_size].i = u;
      m->pairs[new_size].j = v;
      new_size++;
    }
  }
  m->pairs_size = new_size;
  m->pairs = (struct matrix_al_pair *)realloc(m->pairs, new_size * sizeof(struct matrix_al_pair));
}

void solver_luby_b_select_step(struct matrix_al *m, struct coloring *c, bool *i_prime) {
  // 3.3 of Luby
  // compute degree of each node
  number_t *degree = calloc(m->n_vertices, sizeof(number_t));
#pragma omp parallel for
  for (size_t i = 0; i < m->n_vertices; i++) {
    for (size_t j = 0; j < m->n_vertices; j++) {
      degree[i] += matrix_al_query(m, i, j) ? 1 : 0;
    }
  }

  // choice step
  bool *x = calloc(m->n_vertices, sizeof(bool));
#pragma omp parallel for
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (random() % 2 == 0) {
      x[i] = false;
    } else {
      x[i] = true;
    }
  }

  // for all edges
  for (size_t i = 0; i < m->pairs_size; i++) {
    number_t u = m->pairs[i].i;
    number_t v = m->pairs[i].j;

    // if u and v are both in x
    if (x[u] && x[v]) {
      if (degree[i] < degree[j]) {
        i_prime[u] = true;
      } else {
        i_prime[v] = true;
      }
    }
  }

  free(degree);
  free(x);
}

number_t solver_luby_b_color(struct matrix_al *m, struct coloring *c, number_t color) {
  // 3.1 of Luby
  bool *i = calloc(m->n_vertices, sizeof(bool));
  struct matrix_al *g_prime = matrix_al_create(m->pairs_size, m->n_vertices, malloc);
  size_t loop_number = 0;
  while (g_prime->pairs_size > 0) {
    printf("loop_number: %zu\n", loop_number);
    bool *i_prime = calloc(m->n_vertices, sizeof(bool));
    solver_luby_b_select_step(m, c, i_prime);
    union_assign(i, i_prime, m->n_vertices);
    bool *y = calloc(m->n_vertices, sizeof(bool));
    union_assign(y, i_prime, m->n_vertices);
    bool *n_i_prime = calloc(m->n_vertices, sizeof(bool));
    make_neighbourhood(g_prime, i_prime, n_i_prime);
    union_assign(y, n_i_prime, m->n_vertices);
    // NOTE: we take a shortcut here and instead of removing the vertices, we just remove the edges that have said vertices
    remove_edges_with(g_prime, y);
    free(i_prime);
    free(y);
    free(n_i_prime);
    loop_number++;
  }
  // i is a maximal independent set now
  // color all the vertices in i with the same color
  number_t vertices_colored = 0;
  for (size_t i = 0; i < m->n_vertices; i++) {
    if (i[i]) {
      c->colors[i] = color;
      vertices_colored++;
    }
  }
  free(i);
  return vertices_colored;
}

void solver_luby_b(struct matrix_al *m, struct coloring *c) {
  number_t vertices_colored = 0;
  while (vertices_colored < m->n_vertices) {
    vertices_colored += solver_luby_b_color(m, c, vertices_colored);
  }
}
