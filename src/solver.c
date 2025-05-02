#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "solver.h"

// === luby_maximal_independent_set implementation ===
// Cite for algorithm implementation: Eric Vigoda, https://faculty.cc.gatech.edu/~vigoda/RandAlgs/MIS.pdf

bool *alloc_make_neighbors(const struct matrix *g, bool *s) {
  bool *neighbors = calloc(g->n_vertices, sizeof(bool));
#pragma omp parallel for shared(neighbors, s, g)
  for (size_t i = 0; i < g->n_vertices; i++) {
    // _OPENMP: inner loop is serial, but inner loop has maximum of max(degree) iterations,
    //          which is expected to be small (<10)
    for (size_t j = g->row_index[i]; j < g->row_index[i + 1]; j++) {
      assert(j < g->nnz);
      size_t u = i;
      size_t v = g->col_index[j];
      if (v >= g->n_vertices || u >= g->n_vertices) {
        printf("===u: %lu, v: %lu\n", u, v);
        printf("===g->col_index[%lu]: %lu\n", j, g->col_index[j]);
        printf("===g->n_vertices: %lu\n", g->n_vertices);
        printf("===g->nnz: %lu\n", g->nnz);
      }
      assert(u < g->n_vertices);
      assert(v < g->n_vertices);
      if (s[u] || s[v]) {
        neighbors[u] = true;
        neighbors[v] = true;
      }
    }
  }
  return neighbors;
}

size_t luby_maximal_independent_set(const struct matrix *g, struct coloring *c, const number_t color, bool *initial_s) {
  assert(c->colors_size == g->n_vertices);
  size_t *degree = calloc(g->n_vertices, sizeof(size_t));
  matrix_degree(g, degree);

  size_t remove_count = 0;

  bool *s = calloc(g->n_vertices, sizeof(bool));
  // G' ← G
  bool *g_prime = calloc(g->n_vertices, sizeof(bool));
#pragma omp parallel for shared(g_prime) reduction(+:remove_count)
  for (size_t i = 0; i < g->n_vertices; i++) {
    if ((c->colors[i] != 0 && c->colors[i] != color) || degree[i] <= 0) {
      g_prime[i] = false;
      remove_count++;
    } else {
      g_prime[i] = true;
    }
  }

#ifdef DEBUG
    printf("remove_count: %lu\n", remove_count);
    printf("g->n_vertices: %lu\n", g->n_vertices);
    size_t g_prime_size = 0;
    for (size_t i = 0; i < g->n_vertices; i++) {
      if (g_prime[i]) {
        g_prime_size++;
      }
    }
    printf("g_prime_size: %lu\n", g_prime_size);
    printf("g->n_vertices minus remove_count: %lu\n", g->n_vertices - remove_count);
    printf("===\n");
#endif

  size_t colored_count = 0;
#ifdef DEBUG
  size_t iter_count = 0;
#endif
  // while G' is not the empty graph
  while (remove_count < g->n_vertices) {
    if (initial_s != NULL) {
      memcpy(s, initial_s, g->n_vertices * sizeof(bool));
      initial_s = NULL;
    } else {
      memset(s, 0, g->n_vertices * sizeof(bool));
      // Choose a random set of vertices S in G' by selecting each vertex v
      // independently with probability 1/(2d(v)).
#pragma omp parallel for shared(s)
      for (size_t i = 0; i < g->n_vertices; i++) {
        if (g_prime[i]) {
          assert(degree[i] > 0);
          if (random() % (2 * degree[i]) == 0) {
            s[i] = true;
          }
        }
      }
    }

    // For every edge (u, v) ∈ E(G') if both endpoints are in S then remove
    // the vertex of lower degree from S (break ties arbitrarily).
    /*struct luby_step2b_arg arg;*/
    /*arg.degree = degree;*/
    /*arg.s = s;*/
    /*arg.g_prime = g_prime;*/
    /*matrix_iterate_edges(g, luby_step2b, &arg);*/
    {
      // _OPENMP: inner loop is serial, but inner loop has maximum of max(degree) iterations,
      //          which is expected to be small (<10)
#pragma omp parallel for shared(s)
      for (size_t i = 0; i < g->n_vertices; i++) {
        for (size_t j = g->row_index[i]; j < g->row_index[i + 1]; j++) {
          size_t u = i;
          size_t v = g->col_index[j];
          // every edge must:
          // - be in G' (membership represented by g_prime), and
          // - have both endpoints in S (membership represented by s)
          if (s[u] && s[v] && g_prime[u] && g_prime[v]) {
            // remove the vertex of lower degree
            if (degree[u] < degree[v]) {
              s[u] = false;
            } else { // tie breaked arbitrarily
              s[v] = false;
            }
          }
        }
      }
    }

    // add S to our independent set
    for (size_t i = 0; i < g->n_vertices; i++) {
      if (s[i]) {
        c->colors[i] = color;
        colored_count++;
      }
    }
    // G' = G'\(S ⋃ neighbors of S), i.e., G' is the induced subgraph
    // on V' \ (S ⋃ neighbors of S) where V' is the previous vertex set.
    bool *is_neighbor = alloc_make_neighbors(g, s);
    for (size_t i = 0; i < g->n_vertices; i++) {
      if ((s[i] || is_neighbor[i]) && g_prime[i]) {
        g_prime[i] = false;
        remove_count++;
      }
    }
    free(is_neighbor);
#ifdef DEBUG
    printf("remove_count: %lu\n", remove_count);
    printf("colored_count: %lu\n", colored_count);
    printf("g->n_vertices: %lu\n", g->n_vertices);
    size_t g_prime_size = 0;
    for (size_t i = 0; i < g->n_vertices; i++) {
      if (g_prime[i]) {
        g_prime_size++;
      }
    }
    assert(g_prime_size == g->n_vertices - remove_count);
    printf("iter_count: %lu\n", iter_count);
    iter_count++;
#endif
  }

  free(s);
  free(g_prime);
  free(degree);
  return colored_count;
}

// === detect_subgraph implementation ===

void array_or(bool *a, bool *b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    a[i] = a[i] || b[i];
  }
}

void array_remove(bool *a, bool *b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    a[i] = a[i] && !b[i];
  }
}

size_t traverse(const struct matrix *g, const size_t u, bool *visited) {
  size_t count = 0;
  size_t *stack = malloc(g->n_vertices * sizeof(size_t));
  size_t stack_size = 0;
  stack[stack_size++] = u;
  visited[u] = true;
  count++;
  while (stack_size > 0) {
    size_t v = stack[--stack_size];
    for (size_t j = g->row_index[v]; j < g->row_index[v + 1]; j++) {
      size_t w = g->col_index[j];
      if (!visited[w]) {
        count++;
        visited[w] = true;
        stack[stack_size++] = w;
      }
    }
  }
  free(stack);
  return count;
}

int qsort_compar(const void *a, const void *b) {
  size_t size_a = *(size_t *) a;
  size_t size_b = *(size_t *) b;
  if (size_a < size_b) {
    return -1;
  } else if (size_a > size_b) {
    return 1;
  } else {
    return 0;
  }
}

int qsort_compar2(const void *a, const void *b, void *arg) {
  size_t *indices = (size_t *) arg;
  size_t size_a = *(size_t *) a;
  size_t size_b = *(size_t *) b;
  if (indices[size_a] < indices[size_b]) {
    return -1;
  } else if (indices[size_a] > indices[size_b]) {
    return 1;
  } else {
    return 0;
  }
}

struct subgraph *detect_subgraph(const struct matrix *g, const size_t k, size_t *subgraphs_length) {
  assert(k >= 2);
  size_t *degree = calloc(g->n_vertices, sizeof(size_t));
  matrix_degree(g, degree);
  struct subgraph *subgraphs = NULL;
  *subgraphs_length = 0;

  size_t *effective_reaches = calloc(g->n_vertices, sizeof(size_t));
  size_t *effective_reach_indices = calloc(g->n_vertices, sizeof(size_t));
  // For every vertex `u` that has a degree less than `k`:
  for (size_t u = 0; u < g->n_vertices; u++) {
    if (degree[u] >= k || degree[u] == 0) {
      continue;
    }

#ifdef DEBUG
    printf("calculate size form vertex %lu\n", u);
    printf("  degree: %lu (k=%lu)\n", degree[u], k);
#endif

    // For each neighbor `v` of the vertex, find the total number of vertices traversable from `v` (excluding `u`).
    size_t n_neighbors = g->row_index[u + 1] - g->row_index[u];
    size_t *reachable_via_neighbor = malloc(n_neighbors * sizeof(size_t));
/*#pragma omp parallel for*/
    for (size_t j = g->row_index[u]; j < g->row_index[u + 1]; j++) {
      size_t v = g->col_index[j];
      bool *visited = calloc(g->n_vertices, sizeof(bool));
      visited[u] = true;
      reachable_via_neighbor[j - g->row_index[u]] = traverse(g, v, visited);
#ifdef DEBUG
      printf("  neighbor %lu: reached %lu vertices\n", v, reachable_via_neighbor[j - g->row_index[u]]);
#endif
      free(visited);
    }

    // Select a subset of vertices that together have less than half the number of vertices in the graph.
    size_t effective_reach, effective_reach_index;
    if (n_neighbors == 1) {
      effective_reach = reachable_via_neighbor[0];
      effective_reach_index = 0;
    } else {
      for (size_t i = 0; i < n_neighbors; i++) {
#ifdef DEBUG
        printf("  reachable_via_neighbor[%lu]: %lu\n", i, reachable_via_neighbor[i]);
#endif
      }
      size_t largest_reach, largest_reach_index, second_largest_reach, second_largest_reach_index;
      if (reachable_via_neighbor[0] > reachable_via_neighbor[1]) {
        largest_reach = reachable_via_neighbor[0];
        largest_reach_index = 0;
        second_largest_reach = reachable_via_neighbor[1];
        second_largest_reach_index = 1;
      } else {
        largest_reach = reachable_via_neighbor[1];
        largest_reach_index = 1;
        second_largest_reach = reachable_via_neighbor[0];
        second_largest_reach_index = 0;
      }
#ifdef DEBUG
      printf("  n_neighbors: %lu\n", n_neighbors);
#endif
      for (size_t j = 2; j < n_neighbors; j++) {
        if (reachable_via_neighbor[j] > largest_reach) {
          second_largest_reach = largest_reach;
          second_largest_reach_index = largest_reach_index;
          largest_reach = reachable_via_neighbor[j];
          largest_reach_index = j;
        } else if (reachable_via_neighbor[j] > second_largest_reach) {
          second_largest_reach = reachable_via_neighbor[j];
          second_largest_reach_index = j;
        }
      }
      effective_reach = second_largest_reach;
      effective_reach_index = second_largest_reach_index;
    }
    free(reachable_via_neighbor);

#ifdef DEBUG
    printf("  effective_reach: %lu\n", effective_reach);
    printf("  effective_reach_index: %lu\n", effective_reach_index);
#endif

    effective_reaches[u] = effective_reach;
    effective_reach_indices[u] = effective_reach_index;
  }

  size_t *sorted_indices = malloc(g->n_vertices * sizeof(size_t));
  for (size_t i = 0; i < g->n_vertices; i++) {
    sorted_indices[i] = i;
  }
  // The smallest subgraphs shall be done first, so larger subgraphs do not prevent those smaller subgraphs from forming.
  qsort_r(sorted_indices, g->n_vertices, sizeof(size_t), qsort_compar2, effective_reaches);

#ifdef DEBUG
  // list out sorted_indices
  printf("sorted_indices:\n");
  for (size_t i = 0; i < g->n_vertices; i++) {
    printf("%02lu ", sorted_indices[i]);
    if (i % 10 == 9) {
      printf("\n");
    }
  }
  printf("\n");
#endif

  bool *used_in_subgraph = calloc(g->n_vertices, sizeof(bool));
  // For every vertex `u` that has a degree less than `k`:
  for (size_t k = 0; k < g->n_vertices; k++) {
    size_t u = sorted_indices[k];
    if (degree[u] >= k || used_in_subgraph[u] || degree[u] == 0) {
      continue;
    }
#ifdef DEBUG
    printf("starting from sorted_indices[%lu]: vertex %lu with expected size %lu\n", k, u, effective_reaches[u]);
#endif
    // create subgraph struct
    struct subgraph new_subgraph = { .vertices = calloc(g->n_vertices, sizeof(bool)) };
    assert(new_subgraph.vertices != NULL);
    memcpy(new_subgraph.vertices, used_in_subgraph, g->n_vertices * sizeof(bool));
#ifdef DEBUG
    printf("  effective_reach_index: %lu\n", effective_reach_indices[u]);
    printf("  g->row_index[u]: %lu\n", g->row_index[u]);
    printf("  g->row_index[u+1]: %lu\n", g->row_index[u + 1]);
#endif
    size_t neighbor = g->col_index[effective_reach_indices[u] + g->row_index[u]];
#ifdef DEBUG
    printf("  neighbor: %lu\n", neighbor);
#endif
    assert(neighbor < g->n_vertices);
    new_subgraph.vertices[u] = true;
#ifdef DEBUG
    size_t traversed = traverse(g, u, new_subgraph.vertices);
    printf("  traversed: %lu\n", traversed);
#else
    traverse(g, u, new_subgraph.vertices);
#endif
    // undo the memcpy above
    array_remove(new_subgraph.vertices, used_in_subgraph, g->n_vertices);
    new_subgraph.vertices[u] = true;

    // make sure the subgraphs form a partition of the entire graph
    array_or(used_in_subgraph, new_subgraph.vertices, g->n_vertices);

    // add to subgraphs list
    subgraphs = realloc(subgraphs, (*subgraphs_length + 1) * sizeof(struct subgraph));
    subgraphs[*subgraphs_length] = new_subgraph;
    *subgraphs_length = *subgraphs_length + 1;
  }
  free(degree);
  free(used_in_subgraph);
  return subgraphs;
}

// === color_cliquelike implementation ===

struct find_initial_constraints_arg {
  size_t *constrained_vertices;
  size_t k;
  size_t filled;
  bool *selection;
};

void find_initial_constraints(number_t u, number_t v, void *data) {
  struct find_initial_constraints_arg *arg = (struct find_initial_constraints_arg *) data;
  size_t *constrained_vertices = arg->constrained_vertices;
  bool *selection = arg->selection;
  if (selection != NULL && !(selection[u] && selection[v])) {
    return;
  }
#define k arg->k
#define filled arg->filled
  assert(k >= 2);
  assert(filled <= k);
  if (filled == k) {
    return;
  }

  if (filled == 0) {
    constrained_vertices[0] = u;
    constrained_vertices[1] = v;
    filled = 2;
  } else {
    bool u_constrained = false;
    bool v_constrained = false;
    for (size_t i = 0; i < filled; i++) {
      if (!u_constrained && constrained_vertices[i] == u) {
        u_constrained = true;
      }
      if (!v_constrained && constrained_vertices[i] == v) {
        v_constrained = true;
      }
    }
    if (u_constrained && v_constrained) {
      // nothing to change
    } else if (u_constrained && !v_constrained) {
      // add v to constrained_vertices
      constrained_vertices[filled] = v;
      filled++;
    } else if (!u_constrained && v_constrained) {
      // add u to constrained_vertices
      constrained_vertices[filled] = u;
      filled++;
    } else if (!u_constrained && !v_constrained) {
      // u and v may be constrained after all, or not...
      // we want to be conservative, and either
      // - wait for other loops to figure this out, or
      // - bail and end with filled < k, which is fine too
    }
  }
#undef k
#undef filled
}

void color_cliquelike(const struct matrix *g, struct coloring *c, const size_t k, bool *selection) {
  assert(c->colors_size == g->n_vertices);
  for (size_t i = 0; i < c->colors_size; i++) {
    c->colors[i] = 0;
  }
  // find initial constraints where results are known to have different colors
  // these constraints will be used to run Luby's in parallel later
  struct find_initial_constraints_arg arg;
  arg.constrained_vertices = malloc(k * sizeof(size_t));
  arg.k = k;
  arg.filled = 0;
  arg.selection = selection;
  matrix_iterate_edges(g, find_initial_constraints, &arg);

  size_t colored_count = 0;

  // color all isolated vertices
  size_t *degree = calloc(g->n_vertices, sizeof(size_t));
  matrix_degree(g, degree);
  for (size_t i = 0; i < g->n_vertices; i++) {
    if (degree[i] == 0) {
      c->colors[i] = 1;
      colored_count++;
    }
  }
  free(degree);

  for (size_t i = 0; i < arg.filled; i ++) {
    printf("  coloring vertex from vertex %lu with color %lu\n", arg.constrained_vertices[i], i+1);
    bool *initial_s = calloc(g->n_vertices, sizeof(bool));
    initial_s[arg.constrained_vertices[i]] = true;
    /*for (size_t i = 0; i < g->n_vertices; i++) {*/
    /*  if (selection != NULL && !selection[i]) {*/
    /*    c->colors[i] = 99;*/
    /*  }*/
    /*}*/
    colored_count += luby_maximal_independent_set(g, c, i+1, initial_s);
    free(initial_s);
  }

  free(arg.constrained_vertices);
  return;
}
