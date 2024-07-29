#!/bin/bash

script="./run.sh"
dir="/home/mytkom/Documents/Obsidian/AliceNotes/Efficiency and Purity calculation/pm_optimal_f1_mixed_002"

for charge in 211; do
    if [ "$charge" -eq 211 ]; then
        particle="pion-plus"
    elif [ "$charge" -eq -211 ]; then
        particle="pion-minus"
    elif [ "$charge" -eq 321 ]; then
        particle="kaon-plus"
    elif [ "$charge" -eq -321 ]; then
        particle="kaon-minus"
    elif [ "$charge" -eq 2212 ]; then
        particle="proton-plus"
    elif [ "$charge" -eq -2212 ]; then
        particle="proton-minus"
    fi

    for certainty in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
      for n_sigma_cut in 3; do
        cert_times_100=$(printf "%.0f" $(echo "$certainty * 100" | bc))
        ${script} ${charge} ${certainty} ${n_sigma_cut} ${particle}-${cert_times_100}-${n_sigma_cut}-$1 "$dir"
      done
    done
done

for charge in 321 2212; do
    if [ "$charge" -eq 211 ]; then
        particle="pion-plus"
    elif [ "$charge" -eq -211 ]; then
        particle="pion-minus"
    elif [ "$charge" -eq 321 ]; then
        particle="kaon-plus"
    elif [ "$charge" -eq -321 ]; then
        particle="kaon-minus"
    elif [ "$charge" -eq 2212 ]; then
        particle="proton-plus"
    elif [ "$charge" -eq -2212 ]; then
        particle="proton-minus"
    fi

    for certainty in 0.01 0.02 0.03 0.04 0.05 0.06 0.07 0.08 0.09 0.1; do
      for n_sigma_cut in 3; do
        cert_times_100=$(printf "%.0f" $(echo "$certainty * 100" | bc))
        ${script} ${charge} ${certainty} ${n_sigma_cut} ${particle}-${cert_times_100}-${n_sigma_cut}-$1 "$dir"
      done
    done
done

chown mytkom "$dir/*" 
