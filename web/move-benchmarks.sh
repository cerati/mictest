#! /bin/bash

# source global variables
source xeon_scripts/common-variables.sh
source xeon_scripts/init-env.sh

# Main output dir name
dir=${1:-benchmarks}

# First collect all plots and text files into common dir
echo "Moving plots and text files locally to ${dir}"
./web/collectBenchmarks.sh ${dir}

# Now copy to lxplus
echo "Moving plots and text files remotely to lxplus"
./web/copyAndSendToLXPLUS.sh ${dir}

# Final cleanup of directory
echo "Removing local files"
./xeon_scripts/trashSKL-SP.sh
rm -rf ${dir}
