#!/bin/bash
 
#SBATCH -J cx4803-final-project-study_weak
#SBATCH -N 1 --ntasks-per-node=4 --cpus-per-task=8
#SBATCH --mem=100G
#SBATCH -t 00:02:00

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
  srun -n $n_tasks ./test_solver_distributed -n $n -nnz $nnz -f slurm-graph-$SLURM_JOB_ID-omp-$n_threads-$n_tasks.dot || echo
}

run 1 1 100 100
run 1 2 201 199
run 1 4 410 410
run 1 8 800 800
run 2 1 201 199
run 4 1 400 400
