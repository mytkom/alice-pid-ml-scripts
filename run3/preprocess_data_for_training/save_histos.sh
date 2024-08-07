#!/bin/bash

script="save_histos.cpp"
subDir=$1
dir="$subDir"

# Create the output directory if it doesn't exist
mkdir -p "$dir"

# Pass the list of .root files to the ROOT script
root -l -b -q "${script}(\"${subDir}\", \"${dir}\")"

