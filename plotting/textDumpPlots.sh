#! /bin/bash

## In case this is run separately from the main script
source xeon_scripts/common-variables.sh
source xeon_scripts/init-env.sh
source val_scripts/init-root.sh

Base_Test="NVU1_NTH1"

for build in BH STD CE # FV
do 
    echo "Making plots from text files for" ${sample} ":" ${build}
    
    for archV in "SNB ${Base_Test}" "SNB NVU8int_NTH24" "KNL ${Base_Test}" "KNL NVU16int_NTH256" "SKL-SP ${Base_Test}" "SKL-SP NVU16int_NTH64"
    do echo ${archV} | while read -r archN archO
	do
	    echo "Extracting plots from dump for" ${archN} ${archO}
	    python plotting/makePlotsFromDump.py ${archN} ${sample} ${build} ${archO}
	done
    done

    echo "Making comparison plots from dump for" ${sample} ":" ${build}
    root -b -q -l plotting/makePlotsFromDump.C\(\"${sample}\",\"${build}\"\)
done
