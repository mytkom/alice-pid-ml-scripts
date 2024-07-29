#!/bin/bash

script="generate_hists.cpp"
histFile="MC_HISTS_LHC18g4_285064_002.root"
aodFile="PID_RESULTS_LHC18g4_285064_002.root"
dir="/wd/alice/alice-pid-ml-scripts/batch_efficiency_and_purity/vec_with_nans_sigmoid_included_in_model_graphs_after_debugging"

mkdir -p "$dir"

for certainty in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for n_sigma_cut in 1 2 3; do
    root -l -b -q "${script}(\"${aodFile}\", \"${histFile}\", ${certainty}, ${n_sigma_cut}, \"${dir}\")"
  done
done

