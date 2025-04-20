[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/UHhs5Uma)
# Final Projects

## Due: EOD, 30 April

## Introduction

This program attempts to do a k-coloring of an arbitrary graph.

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

### Algorithms

TODO: serial algorithm, general consturction

TODO: Luby

## Methods

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
