#! /bin/bash

# in case this is called separately
source xeon_scripts/common-variables.sh
source xeon_scripts/init-env.sh

# execute SNB tests remotely
echo "Executing SNB tests remotely..."
SSHO ${SNB_HOST} bash -c "'
cd ${SNB_WORKDIR}/${SNB_TEMPDIR}
./xeon_scripts/benchmark-cmssw-ttbar-fulldet-build.sh SNB
exit
'"

# copy logs back for plotting
echo "Copying logs back from SNB for plotting"
scp ${SNB_HOST}:${SNB_WORKDIR}/${SNB_TEMPDIR}/log_SNB_${sample}_*.txt .

# destroy tmp files
./xeon_scripts/trashSNB.sh
