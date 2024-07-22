#!/bin/bash

ALIBUILD_WORK_DIR="/home/wildjellybear/alice/sw"
VIRTUAL_ENV_ACTIVATE="/home/wildjellybear/.virtualenvs/pdi/bin/activate"

export PIDML_TRAINING_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $ALIBUILD_WORK_DIR
alienv setenv O2Physics/latest -c $PIDML_TRAINING_DIR/scripts/download-multiple-grid-data.sh
alienv setenv O2Physics/latest -c $PIDML_TRAINING_DIR/scripts/run-pidml-mc-producer.sh

CSV_FILE=preprocessed_ao2ds.csv

source $VIRTUAL_ENV_ACTIVATE

cd $PIDML_TRAINING_DIR
# root to csv
$PIDML_TRAINING_DIR/scripts/preprocessing.py $PIDML_TRAINING_DIR/data/preprocessed_ao2ds.root -o $PIDML_TRAINING_DIR/data/$CSV_FILE
# data preparation for models
$PIDML_TRAINING_DIR/scripts/prepare_data.py $PIDML_TRAINING_DIR/data/$CSV_FILE -o $PIDML_TRAINING_DIR/data/processed
# training
$PIDML_TRAINING_DIR/scripts/train.py -c $PIDML_TRAINING_DIR/scripts/train_default_cfg.json
# export to onnx
$PIDML_TRAINING_DIR/scripts/export_onnx.py 
exit

# TODO: training
# TODO: data clean up
# TODO: run benchmark task (generate graphs)
