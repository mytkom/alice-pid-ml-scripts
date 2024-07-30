#!/bin/bash

# ALICE::FZK::SE (default) takes ages to load or crash
export alien_CLOSE_SE=ALICE::GSI::SE2

config_file="my-dpl-config.json"
AO2DS_DIR=/wd/alice/AODs
FILENAME=alice-sim-2024-LHC24b1b-0-528461-AOD-034-AO2D

o2-analysis-timestamp --configuration json://$config_file -b |
o2-analysis-event-selection --configuration json://$config_file -b |
o2-analysis-trackselection --configuration json://$config_file -b |
o2-analysis-multiplicity-table --configuration json://$config_file -b |
o2-analysis-track-propagation --configuration json://$config_file -b |
o2-analysis-pid-tof-base --configuration json://$config_file -b |
o2-analysis-pid-tof-beta --configuration json://$config_file -b |
o2-analysis-pid-tof-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-base --configuration json://$config_file -b |
o2-analysis-pid-ml-batch-eff-and-pur-producer --severity debug --aod-file $AO2DS_DIR/$FILENAME.root --configuration json://$config_file -b \
  --aod-writer-keep AOD/PIDEFFANDPURRES/0:::PID_RESULTS_$FILENAME
mv AnalysisResults.root MC_HISTS_$FILENAME.root
