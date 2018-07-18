#! /bin/bash

# in case this is called separately
source xeon_scripts/common-variables.sh
source xeon_scripts/init-env.sh

# execute knl tests remotely
echo "Executing KNL tests remotely..."
SSHO ${KNL_HOST} bash -c "'
cd ${KNL_WORKDIR}/${KNL_TEMPDIR}
./xeon_scripts/benchmark-cmssw-ttbar-fulldet-build.sh KNL
exit
'"

# copy logs back for plotting
echo "Copying logs back from KNL for plotting"
scp ${KNL_HOST}:${KNL_WORKDIR}/${KNL_TEMPDIR}/log_KNL_${sample}_*.txt .

# destroy tmp files
./xeon_scripts/trashKNL.sh
