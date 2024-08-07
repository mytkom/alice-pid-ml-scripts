#!/bin/bash

script="processHistograms.cpp"
dir="$3"

# Create the output directory if it doesn't exist
mkdir -p "$dir"

# Pass the list of .root files to the ROOT script
root -l -b -q "${script}(\"$1\", \"$2\", \"${dir}\")"
