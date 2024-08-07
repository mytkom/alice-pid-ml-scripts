#!/bin/bash

# ALICE::FZK::SE (default) takes ages to load or crash
export alien_CLOSE_SE=ALICE::GSI::SE2

config_file="pidml-producer-config.json"
AO2DS_DIR=/wd/alice/AODs/
# FILENAME="@/wd/alice/alice-pid-ml-scripts/run3/training/data/local_ao2ds_list.txt"
RESULT_NAME="LHC24b1b_9_AODS_MC_Q_SPLIT"

o2-analysis-event-selection --configuration json://$config_file -b |
o2-analysis-timestamp --configuration json://$config_file -b |
o2-analysis-multiplicity-table --configuration json://$config_file -b |
o2-analysis-track-propagation --configuration json://$config_file -b |
o2-analysis-trackselection --configuration json://$config_file -b |
o2-analysis-pid-tof-base --configuration json://$config_file -b |
o2-analysis-pid-tof-beta --configuration json://$config_file -b |
o2-analysis-pid-tof-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-base --configuration json://$config_file -b |
o2-analysis-pid-tpc --configuration json://$config_file -b |
o2-analysis-pid-ml-producer --configuration json://$config_file -b \
  --aod-writer-keep AOD/PIDTRACKSMCML/0:::MC_ALL_AO2D_$RESULT_NAME
mv AnalysisResults.root MC_RESULTS_$RESULT_NAME.root
