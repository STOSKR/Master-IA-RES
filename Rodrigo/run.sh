#!/bin/bash
#SBATCH -p long
#SBATCH --cpus-per-task=8
#SBATCH --job-name=htr_train
#SBATCH --mem=8G
#SBATCH --gres=shard:1
#SBATCH -o logs/%j.log

source /opt/miniconda3/etc/profile.d/conda.sh
conda activate RFA2526pt

# Configure CUDA memory management
export PYTORCH_ALLOC_CONF=expandable_segments:True

# Ensure required Python dependency is available in the active environment.
# Installing the full requirements here can fail on newer Python versions
# because of strict legacy pins (e.g. numpy==1.24.1 on Python 3.12).
if ! python -c "import termcolor" >/dev/null 2>&1; then
    echo "Missing Python dependency detected (termcolor). Installing only termcolor..."
    python -m pip install termcolor
    if [ $? -ne 0 ]; then
        echo "WARNING: Could not install termcolor automatically."
        echo "Training will continue; output colors may be disabled."
    fi
fi

# Ensure we use only the GPU assigned by SLURM
if [ ! -z "$CUDA_VISIBLE_DEVICES" ]; then
    echo "Using GPU(s): $CUDA_VISIBLE_DEVICES"
fi

# Print GPU memory info
nvidia-smi --query-gpu=index,name,memory.total,memory.free --format=csv

echo "=========================================="
echo "Starting HTR training: $(date)"
echo "=========================================="

# Ensure output directory exists for model and training log files.
mkdir -p models/Optical

python HTRnn/train.py \
--models-file data/lists/symbols_train.lst \
--data-augm --epochs 120 \
--space-symbol "~" --batch-size 15 \
data/data_train data/data_val/ models/Optical/rodrigo.pth

if [ $? -eq 0 ]; then
    echo "=========================================="
    echo "Training completed successfully!"
    echo "=========================================="
else
    echo "=========================================="
    echo "ERROR: Training failed."
    echo "=========================================="
    exit 1
fi