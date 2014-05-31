#!/bin/bash
#SBATCH -J cilkRandom
#SBATCH -o stdout.%j
#SBATCH -e stderr.%j
#SBATCH -n 1
#SBATCH -t 16:00:00
#SBATCH -p normal
#SBATCH --mail-user=mhinson@umail.ucsb.edu
#SBATCH --mail-type=all
python client.py