#!/bin/bash
 
#SBATCH -J cx4803-final-project-correctness_check
#SBATCH -N 1 --ntasks-per-node=1 --cpus-per-task=8
#SBATCH --mem=100G
#SBATCH -t 00:00:30
 
set -euxo pipefail

make clean test_solver_color
time ./test_solver_color -n 50000 -nnz 50000 -f slurm-graph-$SLURM_JOB_ID.dot
echo $?
