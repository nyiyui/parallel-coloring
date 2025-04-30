#pragma once
#include "graph.h"

struct subgraph {
    bool *vertices;
};

size_t luby_maximal_independent_set(struct matrix *g, struct coloring *c, number_t color, bool *initial_s);

struct subgraph *detect_subgraph(struct matrix *g, size_t k, size_t *subgraphs_length);

void color_cliquelike(struct matrix *g, struct coloring *c, size_t k);
