#!/bin/bash
#SBATCH -J ramseyCilkRandom
#SBATCH -o stdout.%j
#SBATCH -e stderr.%j
#SBATCH -n 1
#SBATCH -t 00:30:00
#SBATCH -p normal
#SBATCH --mail-user=mhinson@umail.ucsb.edu
#SBATCH --mail-type=all
./simple_taboo_search-6