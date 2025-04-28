#pragma once
#include "graph.h"

size_t luby_maximal_independent_set(struct matrix *g, struct coloring *c, number_t color, bool *initial_s);

void color_cliquelike(struct matrix *g, struct coloring *c, size_t k);
