# Contributing

Thanks for helping maintain this club project. Keep changes small, documented, and easy for the next person to run.

## Development Setup

1. Create a virtual environment:

   ```bash
   python3 -m venv .venv
   source .venv/bin/activate
   ```

2. Install dependencies:

   ```bash
   pip install -r requirements.txt
   ```

3. Run the sample image classifier:

   ```bash
   python3 scripts/test.py
   ```

## Project Rules

- Keep the main runnable scripts in `scripts/test.py` and `scripts/snapshot.py`.
- Keep the current trained model at `dataset/model/waste_model_1.pth`.
- Keep small sample images in `dataset/sample/`.
- Keep Arduino sketches in `arduino/`.
