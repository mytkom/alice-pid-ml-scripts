#!/usr/bin/env python

import pandas as pd
import sys
import wandb
import os
import json
import torch
import torch.nn as nn

def main(cfg_file: str = None, save_dir: str = None):
    if cfg_file is None:
        cfg_file = 'scripts/train_default_cfg.json'
    if save_dir is None:
        save_dir = 'models/'
    f = open(cfg_file)
    cfg = json.load(f)

    config_common = cfg["common"]

    # torch.cuda.set_device(0)
    torch.multiprocessing.set_sharing_strategy('file_system')
    device = torch.device("cuda")

    from pdi.data.preparation import FeatureSetPreparation, MeanImputation, DeletePreparation, RegressionImputation, \
        EnsemblePreparation
    from pdi.models import AttentionModel, NeuralNetEnsemble, NeuralNet
    from pdi.data.constants import N_COLUMNS

    MODEL = {
            "data_preparation":
                FeatureSetPreparation(),
            "config": {
                "embed_in": N_COLUMNS + 1,
                "embed_hidden": 128,
                "d_model": 32,
                "ff_hidden": 128,
                "pool_hidden": 64,
                "num_heads": 2,
                "num_blocks": 2,
                "start_lr": 2e-4,
            },
            "model_class":
                AttentionModel,
            "model_args":
                lambda d_prep: [
                    wandb.config.embed_in,
                    wandb.config.embed_hidden,
                    wandb.config.d_model,
                    wandb.config.ff_hidden,
                    wandb.config.pool_hidden,
                    wandb.config.num_heads,
                    wandb.config.num_blocks,
                    nn.ReLU,
                    wandb.config.dropout,
                ],
        }

    do_train(device, save_dir, config_common, **MODEL)


def do_train(device, save_dir, config_common,  data_preparation, config, model_class,
             model_args):
    from pdi.train import train
    from pdi.constants import (
            PARTICLES_DICT,
            TARGET_CODES,
            NUM_WORKERS,
        )
    from pdi.data.types import Split
    wandb_config = {**config_common, **config}

    train_loader, val_loader = data_preparation.prepare_dataloaders(
        wandb_config["bs"], NUM_WORKERS, [Split.TRAIN, Split.VAL])

    thresholds_df_list = []

    for target_code in TARGET_CODES:
        save_path = f"{save_dir}/{PARTICLES_DICT[target_code]}.pt"
        with wandb.init(project="Proposed",
                        config=wandb_config,
                        name=PARTICLES_DICT[target_code],
                        anonymous="allow") as run:
            # pos_weight = torch.tensor(data_preparation.pos_weight(target_code)).float().to(device)
            pos_weight = torch.tensor(1.0).to(device)
            wandb.log({"pos_weight": pos_weight.item()})

            model_init_args = model_args(data_preparation)
            model = model_class(*model_init_args).to(device)

            os.makedirs(f"{save_dir}/", exist_ok=True)
            train(model, target_code, device, train_loader, val_loader,
                  pos_weight)

            save_dict = {
                "state_dict": model.state_dict(),
                "model_args": model_init_args,
                "model_thres": model.thres
            }

            thresholds_df_list.append(pd.DataFrame([(target_code, model.thres)], columns=["pdgPid", "threshold"]))

            torch.save(save_dict, save_path)

    thresholds_df = pd.concat(thresholds_df_list, ignore_index=True)
    thresholds_df.to_csv(f"{save_dir}/thresholds.csv", index=False)

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train models.")
    parser.add_argument('-p', '--pdi-dir', dest="pdi_dir", type=str, help="Path to directory containing pdi source code")
    parser.add_argument('-c', '--config', dest="cfg_file", type=str, help="Configuration file.")
    parser.add_argument('-s', '--save-dir', dest="save_dir", type=str, help="Save directory for models.")
    args = parser.parse_args()

    if args.pdi_dir not in sys.path:
        sys.path.append(args.pdi_dir)

    main(args.cfg_file, args.save_dir)
