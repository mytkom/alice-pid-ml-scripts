#!/bin/bash

# ALICE::FZK::SE (default) takes ages to load or crash
export alien_CLOSE_SE=ALICE::GSI::SE2

config_file="ml-mc-config.json"
FILENAME=LHC18g4-9-AODs

o2-analysis-timestamp --configuration json://$config_file -b |
o2-analysis-tracks-extra-converter --configuration json://$config_file -b |
o2-analysis-trackextension --configuration json://$config_file -b |
o2-analysis-trackselection --configuration json://$config_file -b |
o2-analysis-bc-converter --configuration json://$config_file -b |
o2-analysis-zdc-converter --configuration json://$config_file -b |
# o2-analysis-centrality-table --configuration json://$config_file -b |
o2-analysis-multiplicity-table --configuration json://$config_file -b |
o2-analysis-collision-converter --configuration json://$config_file -b |
o2-analysis-pid-tof-base --configuration json://$config_file -b |
o2-analysis-pid-tof-beta --configuration json://$config_file -b |
o2-analysis-pid-tof-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-base --configuration json://$config_file -b |
o2-analysis-pid-ml-producer-mc --configuration json://$config_file -b \
  --aod-writer-keep AOD/PIDTRACKSMCML/0:::MC_ALL_AO2D_$FILENAME
mv AnalysisResults.root MC_RESULTS_$FILENAME.root
