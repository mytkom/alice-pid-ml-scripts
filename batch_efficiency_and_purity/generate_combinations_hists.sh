#!/bin/bash

script="generate_hists.cpp"
histFile="MC_HISTS_AO2D_18G4_run285064_023.root"
aodFile="PID_RESULTS_AO2D_18G4_run285064_023.root"
dir="/home/mytkom/Documents/Obsidian/AliceNotes/Efficiency and Purity calculation/pm_mixed_on_dataset_002"

for certainty in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for n_sigma_cut in 1 2 3; do
    root -l -b -q "${script}(\"${aodFile}\", \"${histFile}\", ${certainty}, ${n_sigma_cut}, \"${dir}\")"
  done
done

# needed locally for me (you can comment out)
chown mytkom "$dir/*" 
