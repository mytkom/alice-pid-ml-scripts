#!/bin/bash

script="save_histos.cpp"
subDir=$1
dir="$subDir"

mkdir -p "$dir"

root -l -b -q "${script}(\"${subDir}\", \"${dir}\")"

