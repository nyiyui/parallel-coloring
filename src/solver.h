#pragma once
#include "graph.h"

size_t luby_maximal_independent_set(const struct matrix *g, struct coloring *c, const number_t color, bool *initial_s);

struct subgraph *detect_subgraph(const struct matrix *g, const size_t k, size_t *subgraphs_length);

void color_cliquelike(const struct matrix *g, struct coloring *c, const size_t k, bool *selection);
