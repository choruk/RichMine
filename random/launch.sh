#!/bin/bash
#SBATCH -J cilkRandom
#SBATCH -o stdout.%j
#SBATCH -e stderr.%j
#SBATCH -n 1
#SBATCH -t 00:30:00
#SBATCH -p normal
#SBATCH --mail-user=mhinson@umail.ucsb.edu
#SBATCH --mail-type=all
./random