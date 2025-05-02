#!/bin/bash

#SBATCH -J cx4803-final-project-study_strong_tasks
#SBATCH -N 4
#SBATCH --mem=100G
#SBATCH -t 00:20:00

# set fake SLURM_JOB_ID if not set
if [ -z "$SLURM_JOB_ID" ]; then
  SLURM_JOB_ID=123456
fi
 
set -euxo pipefail

make clean test_solver_distributed
function run {
  n_threads=$1
  n_tasks=$2
  n=$3
  nnz=$4
  echo "n_threads=$n_threads"
  echo "n_tasks=$n_tasks"
  echo "n=$n"
  echo "nnz=$nnz"
  export OMP_NUM_THREADS=$n_threads
  export OMP_PROC_BIND=true
  export OMP_PLACES=cores
  export OMP_THREAD_LIMIT=$n_threads
  export OMP_DYNAMIC=false
  srun $SRUN_OPT -n $n_tasks ./test_solver_distributed -n $n -nnz $nnz -f slurm-graph-$SLURM_JOB_ID-omp-$n_threads-$n_tasks.dot || echo
}

for n_threads in 1 2 4 8; do
  for n_tasks in 1 2 4 8 16 32; do
    n=10000
    export SRUN_OPT="--cpus-per-task=$n_threads --ntasks=$n_tasks"
    run $n_threads $n_tasks $n $n
  done
done
