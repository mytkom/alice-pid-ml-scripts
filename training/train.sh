#!/bin/bash

ALIBUILD_WORK_DIR="/root/alice"
VIRTUAL_ENV_ACTIVATE="/home/mytkom/.virtualenvs/myVEnv/bin/activate"

export PIDML_TRAINING_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $ALIBUILD_WORK_DIR
alienv setenv O2Physics/latest -c $PIDML_TRAINING_DIR/scripts/download-multiple-grid-data.sh
alienv setenv O2Physics/latest -c $PIDML_TRAINING_DIR/scripts/run-pidml-mc-producer.sh

source $VIRTUAL_ENV_ACTIVATE
$PIDML_TRAINING_DIR/scripts/preprocessing.py $PIDML_TRAINING_DIR/data/preprocessed_ao2ds.root -o $PIDML_TRAINING_DIR/data/preprocessed_ao2ds.csv 
exit

# TODO: training
# TODO: data clean up
# TODO: run benchmark task (generate graphs)
