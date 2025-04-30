#!/bin/bash
 
#SBATCH -J cx4803-final-project-test_all
#SBATCH -N 1 --ntasks-per-node=1 --cpus-per-task=8
#SBATCH --mem=100G
#SBATCH -t 00:10:00
 
set -euxo pipefail

make clean test_all
