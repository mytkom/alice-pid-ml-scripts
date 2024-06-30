#!/bin/bash

script="./run.sh"
dir="/home/mytkom/Documents/Obsidian/AliceNotes/Efficiency and Purity calculation/Proposed Model Graphs"

for charge in 211 -211 321 -321 2212 -2212; do
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

    for value in 0.4 0.5 0.6; do
        val_times_100=$(printf "%.0f" $(echo "$value * 100" | bc))
        $script $charge $value ${particle}-${val_times_100}-$1 "$dir"
    done
done

chown $dir/* mytkom
