#!/bin/bash
#SBATCH -Jmoonrobot-arm                # Job name
#SBATCH -N1 --cpus-per-task=4                    # Number of nodes and CPUs per node required
#SBATCH --mem-per-cpu=4G                         # Memory per core
#SBATCH -t 00:30:00                              # Duration of the job (Ex: 30 mins)
#SBATCH -p rg-nextgen-hpc                        # Partition Name
#SBATCH -o ./slurm-arm.out
#SBATCH -C aarch64,ampereq8030                   # Request an ARM64 node
#SBATCH -W                                       # Do not exit until the submitted job terminates.

## Print the job environment
cd $GITHUB_WORKSPACE
echo "Working Directory: "$(pwd)
echo "#####################################################################"
echo "Running on $(hostname) in the CRNCH Rogues Gallery"
echo "#####################################################################"

## Build the Project
make SIMULATION=native prep
make
make install
