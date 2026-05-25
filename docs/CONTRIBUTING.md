# Contributing

Thanks for helping maintain this club project. These guidelines help keep the project organized and prevent common issues.

## Project Rules

- Keep local training and testing scripts in `scripts/`.
- Use `scripts/sample_img_test.py` for sample image testing.
- Use `scripts/webcam_test.py` for webcam testing.
- Use `scripts/train.py` for model training.
- Keep the Raspberry Pi runtime code in `raspberry_pi/main.py`. The script already runs on boot on the Pi, so update the copy on the Pi when this file changes.
- Keep the best trained model at `dataset/model/waste_model_1.pth`. When replacing the model, also update the copy on the Raspberry Pi using file transfer or USB.
- Keep small sample images in `dataset/sample/`. More sample images = local testing easier.
- Keep the full dataset archives at `dataset/train.zip` and `dataset/test.zip`. DO NOT COMMIT UNZIPPED DATASET folders because they take too much storage.
- Keep Arduino sketches in `arduino/` but of course, update as needed on the ardunio IDE. 
