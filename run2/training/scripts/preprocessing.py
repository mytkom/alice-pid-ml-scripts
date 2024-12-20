#!/usr/bin/env python

import pandas as pd
import numpy as np
import uproot3
import matplotlib.pyplot as plt
import os
import argparse
from matplotlib import colors

def main(input_files, output_filepath):
    dataframes = []
    for input_file in input_files:
        file = uproot3.open(input_file)
        for dirname in file:
            dirname = dirname.decode("utf-8")
            pure_dirname = dirname.split(";")[0]
            if pure_dirname.startswith("DF_"):
                tree_data = file["%s/O2pidtracksmcml" % (dirname)].pandas.df()
                dataframes.append(tree_data)

    data = pd.concat(dataframes, ignore_index=True)
    print(data.head())
    print(data.columns)

    # p = np.sqrt(data.fPx ** 2 + data.fPy ** 2 + data.fPz ** 2)
    # data["P"] = p
    data["fBeta"].mask(np.isclose(data["fBeta"], -999), inplace=True)
    data["fTOFSignal"].mask(np.isclose(data["fTOFSignal"], -999), inplace=True)
    data["fTRDPattern"].mask(np.isclose(data["fTRDPattern"], 0), inplace=True)
    # data["fTRDSignal"].mask(np.isclose(data["fTRDSignal"], -999), inplace=True)
    data = data[data["fTPCSignal"] > 0]

    data.to_csv(output_filepath)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process ROOT files.")
    parser.add_argument('input_files', nargs='+', help="List of input ROOT files.")
    parser.add_argument('-o', '--output', dest="output", help="output name")
    args = parser.parse_args()
    
    main(args.input_files, args.output)

