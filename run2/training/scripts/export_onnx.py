#!/usr/bin/env python

import sys
import os
import argparse
import torch
import onnx

def main(input_dir: str, output_dir: str):
    if input_dir is None:
        input_dir = "models"
    if output_dir is None:
        output_dir = "onnx"
    from pdi.constants import (
        PARTICLES_DICT,
        TARGET_CODES
    )
    from pdi.data.constants import GROUP_ID_KEY
    from pdi.data.preparation import DeletePreparation, FeatureSetPreparation
    from pdi.models import NeuralNet, AttentionModel
    from pdi.data.types import Split

    device = torch.device("cpu")

    data_preparation = FeatureSetPreparation()
    (train_loader, ) = data_preparation.prepare_dataloaders(1, 0, [Split.TEST])

    input_data, _, data_dict = next(iter(train_loader))
    gid = data_dict.get(GROUP_ID_KEY)

    dummy_input = input_data.to(device)
    print("Example data shape for exporting:")
    print(list(dummy_input.shape))

    os.makedirs(f"{output_dir}", exist_ok=True)
    for target_code in TARGET_CODES:
        load_path = f"{input_dir}/{PARTICLES_DICT[target_code]}.pt"
        export_path = f"{output_dir}/{PARTICLES_DICT[target_code]}.onnx"
        saved_model = torch.load(load_path)
        model = AttentionModel(*saved_model["model_args"]).to(device)
        model.thres = saved_model["model_thres"]
        model.load_state_dict(saved_model["state_dict"])

        torch.onnx.export(model, dummy_input, export_path)

        onnx_model = onnx.load(export_path)
        onnx.checker.check_model(onnx_model)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process CSV file.")
    parser.add_argument('-p', '--pdi-dir', dest="pdi_dir", type=str, help="Path to directory containing pdi source code")
    parser.add_argument('-i', '--input', type=str, dest="input_dir", help="Directory name with .pt model for all particles")
    parser.add_argument('-o', '--output', type=str, dest="output_dir", help="Output dir name.")
    args = parser.parse_args()

    if args.pdi_dir not in sys.path:
        sys.path.append(args.pdi_dir)

    main(args.input_dir, args.output_dir)
