#!/bin/bash

# ALICE::FZK::SE (default) takes ages to load or crash
export alien_CLOSE_SE=ALICE::GSI::SE2

config_file="my-dpl-config.json"
usage="$0 <pid> <certainty> <nSigmaCut> <output graph filename> <output directory>"

sed -i 's/"pid": "\-\?[0-9]*",$/"pid": "'"$1"'",/' $config_file
sed -i 's/"certainty": "[0-9\.]*",$/"certainty": "'"$2"'",/' $config_file
sed -i 's/"n-sigma-cut": "[0-9\.]*",$/"n-sigma-cut": "'"$3"'",/' $config_file

o2-analysis-tracks-extra-converter --configuration json://$config_file -b |
o2-analysis-timestamp --configuration json://$config_file -b |
o2-analysis-trackextension --configuration json://$config_file -b |
o2-analysis-trackselection --configuration json://$config_file -b |
o2-analysis-multiplicity-table --configuration json://$config_file -b |
o2-analysis-bc-converter --configuration json://$config_file -b |
o2-analysis-collision-converter --configuration json://$config_file -b |
o2-analysis-zdc-converter --configuration json://$config_file -b |
o2-analysis-pid-tof-base --configuration json://$config_file -b |
o2-analysis-pid-tof-beta --configuration json://$config_file -b |
o2-analysis-pid-tof-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-full --configuration json://$config_file -b |
o2-analysis-pid-tpc-base --configuration json://$config_file -b |
o2-analysis-pid-ml-eff-and-pur-producer --configuration json://$config_file -b

if [ "$?" -eq "0" ] ; then root -q -b 'efficiency_and_purity.c("AnalysisResults.root", "'"$4"'", "'"$5"'")'; fi
