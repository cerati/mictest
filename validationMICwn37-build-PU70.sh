#! /bin/bash

make -j 12 WITH_ROOT=yes

fin=/store/disk00/slava77/analysis/CMSSW_9_1_0_pre1-tkNtuple/run1000/2017/pass-4874f28/initialStep/PU70/10224.0_TTbar_13+TTbar_13TeV_TuneCUETP8M1_2017PU_GenSimFullINPUT+DigiFullPU_2017PU+RecoFullPU_2017PU+HARVESTFullPU_2017PU/memoryFile.fv3.clean.writeAll.recT.071817.bin

base=SNB_CMSSW_PU70_clean

for sV in "sim " "see --cmssw-seeds"; do echo $sV | while read -r sN sO; do
        for bV in "BH bh" "STD std" "CE ce"; do echo $bV | while read -r bN bO; do
		oBase=${base}_${sN}_${bN}
		nTH=8
		echo "${oBase}: validation [nTH:${nTH}, nVU:8]"
		./mkFit/mkFit --geom CMS-2017 --root-val --read --file-name ${fin} --build-${bO} ${sO} --num-thr ${nTH} >& log_${oBase}_NVU8int_NTH${nTH}_val.txt
		mv valtree.root valtree_${oBase}.root
            done
        done
    done
done

make clean

for opt in sim see
do
    oBase=${base}_${opt}
    for build in BH STD CE
    do
	root -b -q -l runValidation.C+\(\"_${oBase}_${build}\"\)
    done
    root -b -q -l makeValidation.C+\(\"${oBase}\"\)
done

make distclean
