#!/bin/bash

script="generate_hists.cpp"
trackSet="All"  # Default value for enum

usage() {
    echo "Usage: $0 <histFile> <aodFile> <dir> [--all | --with-tof | --without-tof]"
    echo ""
    echo "Arguments:"
    echo "  histFile   Path to the histogram file."
    echo "  aodFile    Path to the AOD file."
    echo "  dir        Directory where output files will be saved."
    echo ""
    echo "Options:"
    echo "  --all          Use 'All' track set (default)."
    echo "  --with-tof     Use 'WithTOF' track set."
    echo "  --without-tof  Use 'WithoutTOF' track set."
    echo ""
    exit 1
}

if [ $# -lt 3 ]; then
    usage
fi

histFile="$1"
aodFile="$2"
dir="$3"
shift 3

while [[ $# -gt 0 ]]; do
    case "$1" in
        --all)
            trackSet="All"
            ;;
        --with-tof)
            trackSet="WithTOF"
            ;;
        --without-tof)
            trackSet="WithoutTOF"
            ;;
        --help|-h)
            usage
            ;;
        *)
            echo "Unknown option $1."
            usage
            ;;
    esac
    shift
done

mkdir -p "$dir"

for certainty in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for n_sigma_cut in 1 2 3; do
    root -l -b -q "${script}(\"${aodFile}\", \"${histFile}\", ${certainty}, ${n_sigma_cut}, \"${dir}\", ${trackSet})"
  done
done

cp "$histFile" "$dir"
cp "$aodFile" "$dir"

