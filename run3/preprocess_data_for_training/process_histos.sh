#!/bin/bash

script="processHistograms.cpp"
dir="$3"

mkdir -p "$dir"

root -l -b -q "${script}(\"$1\", \"$2\", \"${dir}\")"
