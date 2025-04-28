#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "solver.h"

// === luby_maximal_independent_set implementation ===
// Cite for algorithm implementation: Eric Vigoda, https://faculty.cc.gatech.edu/~vigoda/RandAlgs/MIS.pdf

struct luby_step2b_arg {
  size_t *degree;
  bool *s;
  const bool *g_prime;
};

static void luby_step2b(number_t i, number_t j, void *data) {
  struct luby_step2b_arg *arg = (struct luby_step2b_arg *)data;
  size_t *degree = arg->degree;
  bool *s = arg->s;
  const bool *g_prime = arg->g_prime;

  // every edge must:
  // - be in G' (membership represented by g_prime), and
  // - have both endpoints in S (membership represented by s)
  if (s[i] && s[j] && g_prime[i] && g_prime[j]) {
    // remove the vertex of lower degree
    if (degree[i] < degree[j]) {
      arg->s[i] = false;
    } else { // tie breaked arbitrarily
      arg->s[j] = false;
    }
  }
}

struct mark_neighbour_arg {
  bool *s;
  bool *neighbours;
};

static void mark_neighbour(number_t i, number_t j, void *data) {
  struct mark_neighbour_arg *arg = (struct mark_neighbour_arg *)data;

  if (arg->s[i] || arg->s[j]) {
    arg->neighbours[i] = true;
    arg->neighbours[j] = true;
  }
}

bool *alloc_make_neighbours(struct matrix *g, bool *s) {
  bool *neighbours = calloc(g->n_vertices, sizeof(bool));
  struct mark_neighbour_arg arg;
  arg.s = s;
  arg.neighbours = neighbours;
  matrix_iterate_edges(g, mark_neighbour, &arg);
  return neighbours;
}

size_t luby_maximal_independent_set(struct matrix *g, struct coloring *c, number_t color, bool *initial_s) {
  assert(c->colors_size == g->n_vertices);
  size_t *degree = calloc(g->n_vertices, sizeof(size_t));
  matrix_degree(g, degree);

  size_t remove_count = 0;

  bool *s = calloc(g->n_vertices, sizeof(bool));
  // G' ← G
  bool *g_prime = calloc(g->n_vertices, sizeof(bool));
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
    struct luby_step2b_arg arg;
    arg.degree = degree;
    arg.s = s;
    arg.g_prime = g_prime;
    matrix_iterate_edges(g, luby_step2b, &arg);

    // add S to our independent set
    for (size_t i = 0; i < g->n_vertices; i++) {
      if (s[i]) {
        c->colors[i] = color;
        colored_count++;
      }
    }
    // G' = G'\(S ⋃ neighbours of S), i.e., G' is the induced subgraph
    // on V' \ (S ⋃ neighbours of S) where V' is the previous vertex set.
    bool *is_neighbour = alloc_make_neighbours(g, s);
    for (size_t i = 0; i < g->n_vertices; i++) {
      if ((s[i] || is_neighbour[i]) && g_prime[i]) {
        g_prime[i] = false;
        remove_count++;
      }
    }
    free(is_neighbour);
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

// === color_cliquelike implementation ===

struct find_initial_constraints_arg {
  struct matrix *g;
  size_t *constrained_vertices;
  size_t k;
  size_t filled;
};

void find_initial_constraints(number_t u, number_t v, void *data) {
  struct find_initial_constraints_arg *arg = (struct find_initial_constraints_arg *) data;
  struct matrix *g = arg->g;
  size_t *constrained_vertices = arg->constrained_vertices;
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

void color_cliquelike(struct matrix *g, struct coloring *c, size_t k) {
  assert(c->colors_size == g->n_vertices);
  for (size_t i = 0; i < c->colors_size; i++) {
    c->colors[i] = 0;
  }
  // find initial constraints where results are known to have different colors
  // these constraints will be used to run Luby's in parallel later
  struct find_initial_constraints_arg arg;
  arg.g = g;
  arg.constrained_vertices = malloc(k * sizeof(size_t));
  arg.k = k;
  arg.filled = 0;
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

  // even though arg.constrained_vertices may not be enough, try anyway:
  for (size_t i = 0; i < arg.filled; i ++) {
    printf("coloring vertex from vertex %lu\n", arg.constrained_vertices[i]);
    bool *initial_s = calloc(g->n_vertices, sizeof(bool));
    initial_s[arg.constrained_vertices[i]] = true;
    colored_count += luby_maximal_independent_set(g, c, i+1, initial_s);
    free(initial_s);
  }

  free(arg.constrained_vertices);
  return;
}
