#!/bin/bash

# ALICE::FZK::SE (default) takes ages to load or crash
export alien_CLOSE_SE=ALICE::GSI::SE2

config_file="$PIDML_TRAINING_DIR/scripts/ml-mc-config.json"
DATA_DIR="$PIDML_TRAINING_DIR/data"
AO2D_LIST_FILE="@$DATA_DIR/local_ao2ds_list.txt"

sed -i 's/"aod-file": ".*"$/"aod-file": "'"${AO2D_LIST_FILE//\//\\/}"'"/' $config_file

o2-analysis-timestamp --configuration json://$config_file -b |
o2-analysis-tracks-extra-converter --configuration json://$config_file -b |
o2-analysis-trackextension --configuration json://$config_file -b |
o2-analysis-trackselection --configuration json://$config_file -b |
o2-analysis-bc-converter --configuration json://$config_file -b |
o2-analysis-zdc-converter --configuration json://$config_file -b |
o2-analysis-centrality-table --configuration json://$config_file -b |
o2-analysis-multiplicity-table --configuration json://$config_file -b |
o2-analysis-collision-converter --configuration json://$config_file -b |
o2-analysis-pid-tof-base --configuration json://$config_file -b |
o2-analysis-pid-tof-beta --configuration json://$config_file -b |
o2-analysis-pid-tof-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-base --configuration json://$config_file -b |
o2-analysis-pid-ml-producer-mc --aod-file $AO2D_LIST_FILE --configuration json://$config_file -b \
  --aod-writer-keep AOD/PIDTRACKSMC/0:::preprocessed_ao2ds --aod-writer-resdir $DATA_DIR
mv AnalysisResults.root $DATA_DIR/producer_task_analysis_results.root
