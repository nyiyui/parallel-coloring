#!/bin/bash
 
#SBATCH -J cx4803-final-project-preliminary_parallel_perf_check
#SBATCH -N 1 --ntasks-per-node=1 --cpus-per-task=32
#SBATCH --mem=100G
#SBATCH -t 00:02:00
 
set -euxo pipefail

make clean test_solver_color_perf
for n_threads in 1 2 4 8 16 32; do
    echo "Running with $n_threads threads"
    export OMP_NUM_THREADS=$n_threads
    export OMP_PROC_BIND=true
    export OMP_PLACES=cores
    export OMP_THREAD_LIMIT=$n_threads
    export OMP_DYNAMIC=false
    time env OMP_THREAD_LIMIT=$n_threads ./test_solver_color_perf -n 50000 -nnz 50000 -f slurm-graph-$SLURM_JOB_ID-omp-$n_threads.dot
    echo $?
done
