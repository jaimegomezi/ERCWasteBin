# ERC Waste Classifier

This repository contains a MobileNetV2-based waste classifier built for the Electronics and Robotics Club at Skyline College (ERC). I used MobileNetV2 because it is a lightweight image classification model designed for mobile and embedded devices, making it a practical choice for running waste classification on a Raspberry Pi. 

## How It Works

My part of the project was to create a computer vision system that classifies waste into three categories: compost, recycle, and trash.

The overall project sorts waste into the correct chute bins, which are placed at different angles. The computer vision system runs on a Raspberry Pi, where it uses the trained model to classify the waste item. After classification, the Raspberry Pi sends a command to the Arduino:

- `1` for compost
- `2` for recycle
- `3` for trash

The Arduino receives the command and tells the motor which position to turn to so the waste goes into the correct bin.

## Development Note

This project was developed over several months with most testing and iteration done locally before the repository was organized. Because of that, the Git commit history does not show every experiment, or model training attempt. The current repository is organized to make future club contributions easier.

## Repository Layout

```text
.
├── dataset/
│   ├── model/
│   │   └── waste_model_1.pth   # Trained model weights
│   └── sample/
│       ├── IMG_6456.jpeg       # Sample image
│       ├── IMG_6457.jpeg       # Sample image
│       └── IMG_6458.jpeg       # Sample image
├── docs/
│   └── CONTRIBUTING.md         # Contributor workflow and project rules
├── arduino/
│   └── waste_sorter/
│       └── waste_sorter.ino    # Stepper motor control sketch
├── scripts/
│   ├── snapshot.py             # Webcam classifier
│   └── test.py                 # Image classifier
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
python3 scripts/test.py
```

## Run Webcam Predictions

Open the webcam preview:

```bash
python3 scripts/snapshot.py
```

Press `SPACE` to classify the current frame. Press `Q` to quit.

## Arduino Motor Control

The Arduino sketch is in:

```text
arduino/waste_sorter/waste_sorter.ino
```

It listens over serial at `9600` baud:

- `1` moves to the compost bin
- `2` moves to the recycle bin
- `3` moves to the trash bin

## Contributing

Read [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) before changing project structure, dependencies, or model files.
