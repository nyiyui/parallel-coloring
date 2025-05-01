#!/bin/bash
 
#SBATCH -J cx4803-final-project-study_weak2
#SBATCH -N 1 --ntasks-per-node=1 --cpus-per-task=32
#SBATCH --mem=100G
#SBATCH -t 00:02:00

# set fake SLURM_JOB_ID if not set
if [ -z "$SLURM_JOB_ID" ]; then
  SLURM_JOB_ID=123456
fi
 
set -euxo pipefail

make clean test_solver_color_perf
function run {
  n_threads=$1
  n=$2
  nnz=$3
  echo "n_threads=$n_threads"
  echo "n=$n"
  echo "nnz=$nnz"
  export OMP_NUM_THREADS=$n_threads
  export OMP_PROC_BIND=true
  export OMP_PLACES=cores
  export OMP_THREAD_LIMIT=$n_threads
  export OMP_DYNAMIC=false
  ./test_solver_color_perf -n $n -nnz $nnz -f slurm-graph-$SLURM_JOB_ID-omp-$n_threads.dot
}

run 1 100 100
run 2 100 100
run 4 100 100
run 8 100 100
run 16 100 100
run 32 100 100
run 2 201 199
run 4 400 400
run 8 800 800
run 16 1600 1600
run 32 3201 3201
