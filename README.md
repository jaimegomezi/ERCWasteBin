# ERC Waste Classifier

This repository contains a MobileNetV2-based waste classifier built for the Electronics and Robotics Club at Skyline College (ERC). I used MobileNetV2 because it is a lightweight image classification model designed for mobile and embedded devices, making it a practical choice for running waste classification on a Raspberry Pi. 

## How It Works

My part of the project was to create a computer vision system that classifies waste into three categories: compost, recycle, and trash.

The overall project sorts waste into the correct chute bins, which are placed at different angles. The computer vision system runs on a Raspberry Pi, where it uses the trained model to classify the waste item. After classification, the Raspberry Pi sends a command to the Arduino:

- `1` for trash
- `2` for recycle
- `3` for compost

It then waits for the Arduino to reply `Done.` before looking for the next item.

The Arduino receives the command and tells the motor which position to turn to so the waste goes into the correct bin.

## Development Note

This project was developed over several months with most testing and iteration done locally before the repository was organized. Because of that, the Git commit history does not show every experiment or model training attempt. The current repository is organized to make future club contributions easier.

## Repository Layout

```text
.
├── dataset/
│   ├── model/
│   │   └── waste_model_1.pth   # Trained model weights
│   ├── sample/
│   │   ├── IMG_6456.jpeg       # Sample image
│   │   ├── IMG_6457.jpeg       # Sample image
│   │   └── IMG_6458.jpeg       # Sample image
│   ├── test.zip                # Test dataset archive
│   └── train.zip               # Training dataset archive
├── docs/
│   ├── CONTRIBUTING.md         # Contributor workflow and project rules
│   └── SETUP_PI.md             # First-time Raspberry Pi setup
├── arduino/
│   └── auto_trashcan/
│       └── auto_trashcan.ino   # Turntable stepper + lid servo sketch
├── raspberry_pi/
│   └── main.py                 # Raspberry Pi runtime script
├── scripts/
│   ├── sample_img_test.py      # Test model on sample images
│   ├── train.py                # Train a model
│   └── webcam_test.py          # Test model with a webcam
└── requirements.txt
```

## Setup

Create and activate a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

Install dependencies:

```bash
pip install -r requirements.txt
```

## Run Image Predictions

Use the included sample images:

```bash
python3 scripts/sample_img_test.py
```

## Run Webcam Predictions

Open the webcam preview:

```bash
python3 scripts/webcam_test.py
```

Press `SPACE` to classify the current frame. Press `Q` to quit.

## Training and Dataset Archives

The full training and test folders are stored as zip files:

```text
dataset/train.zip
dataset/test.zip
```

Unzip them before running training or dataset tests again:

```bash
unzip dataset/train.zip -d dataset/
unzip dataset/test.zip -d dataset/
```

Then run the training script:

```bash
python3 scripts/train.py
```

## Raspberry Pi Runtime

First-time setup is in [docs/SETUP_PI.md](docs/SETUP_PI.md). The Raspberry Pi script is:

```bash
python3 raspberry_pi/main.py
```

## Arduino Motor Control

The Arduino sketch is in:

```text
arduino/auto_trashcan/auto_trashcan.ino
```

It listens over serial at `9600` baud:

- `1` moves to the trash section
- `2` moves to the recycle section
- `3` moves to the compost section

After rotating, it opens the lid, closes it, and prints `Done.`

## Contributing

Read [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) before changing project structure, dependencies, or model files.
