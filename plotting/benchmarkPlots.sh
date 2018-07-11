#! /bin/bash

## In case this is run separately from the main script
source xeon_scripts/common-variables.sh
source xeon_scripts/init-env.sh
source val_scripts/init-root.sh

##### Make benchmark plots for each architecture #####
for archV in "SNB snb" "KNL knl" "SKL-SP skl-sp"
   do echo ${archV} | while read -r archN archO
	do
	echo "Extract benchmarking results for" ${archN}
	python plotting/makeBenchmarkPlots.py ${archN} ${sample}

	echo "Make final plot comparing different build options for" ${archN}
	root -b -q -l plotting/makeBenchmarkPlots.C\(\"${archN}\",\"${sample}\"\)	

	for build in CE FV
	do
	    echo "Extract multiple events in flight benchmark results for" ${build} "on" ${archN}
	    python plotting/makeMEIFBenchmarkPlots.py ${archN} ${sample} ${build}
	    
	    echo "Make final plot comparing multiple events in flight for" ${build} "on" ${archN}
	    root -b -q -l plotting/makeMEIFBenchmarkPlots.C\(\"${archN}\",\"${sample}\",\"${build}\"\)	
	done
    done
done
