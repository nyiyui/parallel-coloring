[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/UHhs5Uma)
# Final Projects

## Due: EOD, 30 April

## Project topics

You will complete one of the parallel programming and analysis projects below. For _all_ project topics, you must address or satisfy all of the following.

- Combine two _different_ parallel programming models: distributed memory (i.e., MPI), shared memory (i.e., OpenMP), GPUs (i.e., CUDA, Kokkos, or OpenMP Offloading).
- Explore different parallelization strategies (i.e., domain decomposition, task-parallelism, data-parallelism, etc.).
- Develop a _verification_ test to ensure the correctness of your solution. Ensure that the solution does not change with the number of parallel tasks.
- Address load balancing and strategies for maintaining balance as tasks are increased.
- Address memory usage and how it scales with tasks for your problem.
- Perform extensive scaling studies (i.e., weak, strong, thread-to-thread speedup) on PACE. Your scaling studies should extend to as large a number of tasks as you are able to with your problem.

Note that for many of these project topics, parallel code can easily be obtained online. _You must develop your own original code to address your problem_. Researching your problem on the web is expected and encouraged, but I recommend you avoid looking directly at someone's code for inspiration.

### 1. Heat Equation

See Section 31.3 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 2. Poisson Equation

See Section 4.2.2 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 3. Conjugate Gradient

See Section 5.5.11 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 4. Gaussian Elimination

See Section 5.1 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 5. Molecular Dynamics

See Chapter 7 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 6. Sorting and Combinatorics

See Chapter 8 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 7. Graph analytics

See Chapter 9 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 8. N-body Simulation

See Chapter 10 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 9. Monte Carlo Methods

See Chapter 11 of [HPSC2020](../refs/EijkhoutIntroToHPC2020.pdf).

### 10. Machine Learning

Open topic and I am happy to discuss if you have ideas! Any reasonable approach to implementing a ML algorithm in parallel, satisfying the criteria laid out above, is good. Some type of model parallelism must be included. Note that ML is broader than neural networks. 

## Project Reports

You will prepare and submit a report detailing your project, code, and results through a github repo. The reports will be graded according to [this rubric](../refs/rubric_project_final.pdf).