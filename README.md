[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/UHhs5Uma)
# Final Projects

## Due: EOD, 30 April

## Introduction

This program attempts to do a k-coloring of a sparse graph.

This project report first explains the motivation of a k-coloring on a sparse graph, then describes some background on the graph and the algorithm used to do the k-coloring.
In the Methods section, the algorithm is described and its correctness is argued. Various implementation details are discussed to allow a reader to replicate the project. Additionally, the methodology used to obtain the results is described.
In the Results section, the results of the project are presented and discussed.
Finally, the Conclusion section summarizes the project and discusses future work.

### Motivation

Parallel (scientific) computing programs and compilers both have a dependency on job dependency, often modeled using a graph structure.

**Example application for register allocation.**
First proposed by [Chaitin et al.](https://doi.org/10.1016%2F0096-0551%2881%2990048-5), vertices represent the live ranges of variables to be allocated.
Live ranges that are live at the same time are connected by an edge.
By a k-coloring, we can effectively assign a register to each variable, such that no two variables that are live at the same time are assigned the same register.

In a "real world" example, a compiler would do this as part or a larger algorithm:
1. Find live ranges of variables by static analysis.
2. Build the graph representing the live ranges and their overlaps.
3. Find a k-coloring of the graph (k is the number of registers). Note that k is usually very small compared to the number of variables (e.g. k=16 for x86-64, k=31 for ARM64).
4. Most likely, not all variables can be mapped to registers, so the compiler will have to put some variables in memory ("spill").

**Example application for job scheduling.**
Each task is represented by a vertex. Each dependency is represented by an edge.
A k-coloring of this graph represents a scheduling of the tasks, such that no two tasks that are dependent on each other are scheduled at the same time.

[Leighton 1979](https://nvlpubs.nist.gov/nistpubs/jres/84/jresv84n6p489_a1b.pdf) has a few other applications:
- exam scheduling in the minimum number of time slots
- storing chemicals in a minimum number of containers (where two chemicals cannot be stored in the same container if they react with each other)

TODO: why sparse graph?

### The Nature of the Graph

The graph is assumed to be sparse, i.e. the number of edges is much smaller than the number of vertices squared.

We assume to take a register allocation graph, and make a few assumptions about the graph:
- THe graph is simple.
- The graph is connected.
- The graph is undirected.

### Independent Set

The independent set of a graph is a set of vertices such that no two vertices in the set are adjacent.
A maximal independent set (MIS) is an independent set that cannot be extended by adding an adjacent vertex.

A MIS of a graph corresponds to a single color in a k-coloring of the graph.

Note that the optimal k-coloring of a graph is NP-hard, so we will not attempt to find the optimal k-coloring. We will just find a k-coloring of the graph.

TODO: serial algorithm, general consturction

Luby's ([Luby 1986](https://courses.csail.mit.edu/6.852/08/papers/Luby.pdf), section 3.2 "Algorithm B") algorithm is a randomized algorithm for finding a maximal independent set of a graph.
The algorithm runs in paralell on a concurrent read exclusive write parallel RAM (CREW PRAM) model.

## Methods

This project was implemented using OpenMP and OpenMPI.
The code generates a random graph, performs a k-coloring of graph (where k large enough to guarantee a coloring), and then verifies that the coloring is correct.
To check that the parallelism is correct, the code runs the serial version of the algorithm and compares the results.
The results are in [DOT](https://graphviz.org/doc/info/lang.html) format, and results are compared by verifying the files are bit-for-bit identical.
Although this method of checking the result is stricter than necessary, it is a good way to ensure that the parallelism is correct.

### Graph Properties

The graph will be given in adjacency matrix format, where the matrix itself is represented in compressed sparse row storage (CSR) format.
Since we assume the graph is not mutated in any step of any algorithm, there is no concern of mutations "locking up" the graph in a parallel algorithm.

Requirements for the graph object (implemented in `./src/graph.h` and `./src/graph.c`):
- Represent a simple graph (`struct matrix`).
- Create a random simple graph of n vertices and m edges (`struct matrix *matrix_create_random(size_t n_vertices, size_t nnz)`).
- Output the graph to dot format (`void matrix_as_dot(struct matrix *m, FILE *f)`).
- Return whether two vertices are adjacent (`bool matrix_query(struct matrix *m, number_t i, number_t j)`).
- Return the degree of each vertex (`void matrix_degree(struct matrix *m, size_t *degree)`).

### Algorithm

Implementation in `./src/solver.h` and `./src/solver.c`.

The algorithm is a trivial application of Luby's maximal independent set algorithm.
The algorithm is as follows (explanation in parentheses):
1. Find the vertex with the largest degree, `v`.
2. Create sets `initial_s[0]` to `initial_s[k-1]` which each (combined) partition the set containing `v` and its neighbors.
3. Run Luby's maximal independent set algorithm on each of the sets `initial_s[i]`.
  - (Note there is no data dependency between the sets, so they can be run in parallel.)
4. Color all isolated vertices (i.e. vertices with degree 0) with an arbitrary color.
5. Done

Correctness:
Since the sets `initial_s[i]` are disjoint, the algorithm is guaranteed to give a valid k-coloring of the graph.

Note that we use a modified Lbuy's algorithm:
1. Construct `degree` such that `degree[v]` is the degree of vertex `v`.
2. Let `g_prime` be the given graph.
3. While `g_prime` is not empty:
  1. Construct set `s` by selecting each vertex from `g_prime` with probability `1/(2*degree[v])`.
  2. For every edge `(u,v)` in `g_prime` and which have both endpoints in `s`, remove the vertex of lower degree from `g_prime` (break ties arbitrarily).
  3. Color all vertices in `s` with our given color.
  4. Remove `s` and its neighbors from `g_prime`.
4. Done

Correctness:
At each stage, we see that `s` is added to the independent set.
Since we remove `s` and its neighbors from `g_prime`, we are guaranteed that our independent set is valid.
Now, once `g_prime` is empty, all vertices have been either colored or was a neighbor of a colored vertex.
Therefore, we are guaranteed that our independent set is valid and maximal when we halt.

> Sufficient info to plausibly replicate
> project with little further info needed.
> Discussion of what exactly was done
> is clear. Discussion of all parameters
> varied clear, and their meaning given.
> Methods clearly appropriate to
> address project goals. [4 pts]

## Results

> Results clearly stated. Reference back
> to project goals given in the
> introduction. Results presented within
> that context. Implications of results
> discussed and explained. [4 pts]

### Preliminary Analysis

`./preliminary_parallel_perf_check.sbatch`

thread-to-thread speedup of the parallel implementation of the MIS algorithm.

| Threads | Time (s) | Efficiency |
|---------|----------|------------|
| 1       | 0.537425 |      1000‰ |
| 2       | 0.318460 |       843‰ |
| 4       | 0.183991 |       730‰ |
| 8       | 0.201795 |       333‰ |
| 16      | 0.188300 |       178‰ |

## Conclusion

> Project goals, methods, and results
> restated. Results put into a broader
> context. Implications of work given,
> and future directions suggested.
> [4 pts]

## Other References

Other references that are not mentioned above are stored here:
- [Luby's Algorithm for Maximal Independent Set](https://www.cs.cmu.edu/afs/cs/academic/class/15750-s17/ScribeNotes/lecture32.pdf)
  - MIS algorithm can be used for finding the maximal coloring of a single color.
- [Luby 1986](https://courses.csail.mit.edu/6.852/08/papers/Luby.pdf)
  - Paper describing Luby's Monte Carlo algorithm for graph coloring.
- https://www.osti.gov/servlets/purl/1246285
- https://ireneli.eu/2015/10/26/parallel-graph-coloring-algorithms/
- https://paralg.github.io/gbbs/docs/benchmarks/covering/graph_coloring/
- https://doi.org/10.1137/0914041

## Project topics

You will complete one of the parallel programming and analysis projects below. For _all_ project topics, you must address or satisfy all of the following.

- Combine two _different_ parallel programming models: distributed memory (i.e., MPI), shared memory (i.e., OpenMP), GPUs (i.e., CUDA, Kokkos, or OpenMP Offloading).
- Explore different parallelization strategies (i.e., domain decomposition, task-parallelism, data-parallelism, etc.).
- Develop a _verification_ test to ensure the correctness of your solution. Ensure that the solution does not change with the number of parallel tasks.
- Address load balancing and strategies for maintaining balance as tasks are increased.
- Address memory usage and how it scales with tasks for your problem.
- Perform extensive scaling studies (i.e., weak, strong, thread-to-thread speedup) on PACE. Your scaling studies should extend to as large a number of tasks as you are able to with your problem.

Note that for many of these project topics, parallel code can easily be obtained online. _You must develop your own original code to address your problem_. Researching your problem on the web is expected and encouraged, but I recommend you avoid looking directly at someone's code for inspiration.

## Project Reports

You will prepare and submit a report detailing your project, code, and results through a github repo. The reports will be graded according to [this rubric](../refs/rubric_project_final.pdf).
