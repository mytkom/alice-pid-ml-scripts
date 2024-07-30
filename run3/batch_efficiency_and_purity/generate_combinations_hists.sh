#!/bin/bash

script="generate_hists.cpp"
histFile="MC_HISTS_alice-sim-2024-LHC24b1b-0-528461-AOD-034-AO2D.root"
aodFile="PID_RESULTS_alice-sim-2024-LHC24b1b-0-528461-AOD-034-AO2D.root"
dir="/wd/alice/alice-pid-ml-scripts/batch_efficiency_and_purity/run3-9-AODs"

mkdir -p "$dir"

for certainty in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for n_sigma_cut in 1 2 3; do
    root -l -b -q "${script}(\"${aodFile}\", \"${histFile}\", ${certainty}, ${n_sigma_cut}, \"${dir}\")"
  done
done

