"""
ERC Waste Sorter - Raspberry Pi runtime

Waits for the IR sensor to see an item on the lid, takes a photo, classifies it
with the MobileNetV2 model, and tells the Arduino which section to rotate to.

Arduino protocol (auto_trashcan.ino, 9600 baud):
    '1' = Trash   '2' = Recycling   '3' = Compost
The Arduino prints "Done." after it has rotated, dumped the item and closed
the lid. This script waits for that before looking for the next item.
"""

import glob
import os
import sys
import time
from pathlib import Path

import cv2
import gpiod
import serial
import torch
from gpiod.line import Direction, Value
from PIL import Image
from torchvision import models, transforms

# ---------------- Settings you may need to change ----------------
PROJECT_ROOT = Path(__file__).resolve().parents[1]
MODEL_PATH = PROJECT_ROOT / "dataset" / "model" / "waste_model_1.pth"

CLASSES = ["compost", "recycle", "trash"]   # must match the order used in training

# What byte to send the Arduino for each class. Matches auto_trashcan.ino.
ARDUINO_COMMAND = {
    "trash":   b"1",
    "recycle": b"2",
    "compost": b"3",
}

SERIAL_PORT = None          # None = auto-detect (/dev/ttyACM0 or /dev/ttyUSB0)
BAUD = 9600

IR_SENSOR_PIN = 17          # BCM numbering (physical pin 11)
# Most IR obstacle modules pull the output LOW when they see something.
# If the script triggers constantly with nothing there, flip this to False.
IR_ACTIVE_LOW = True

CAMERA_INDEX = 0
CAPTURE_DELAY = 0.5         # let the item settle before the photo (s)
MIN_CONFIDENCE = 0.50       # below this, play it safe and send to trash
ARDUINO_TIMEOUT = 20.0      # max seconds to wait for the Arduino's "Done."
COOLDOWN = 1.0              # pause after each item (s)
# ------------------------------------------------------------------


def find_serial_port():
    if SERIAL_PORT:
        return SERIAL_PORT
    ports = sorted(glob.glob("/dev/ttyACM*")) + sorted(glob.glob("/dev/ttyUSB*"))
    if not ports:
        sys.exit("ERROR: No Arduino found. Is the USB cable plugged into the Pi?")
    return ports[0]


def find_gpio_chip():
    """Pi 4 and newer Pi 5 kernels use gpiochip0; older Pi 5 kernels use
    gpiochip4. Pick whichever chip drives the 40-pin header."""
    for path in sorted(glob.glob("/dev/gpiochip*")):
        try:
            with gpiod.Chip(path) as chip:
                label = chip.get_info().label
        except OSError:
            continue
        if label.startswith("pinctrl-rp1") or label.startswith("pinctrl-bcm2"):
            return path
    return "/dev/gpiochip0"


def wait_for_line(ser, text, timeout):
    """Read lines from the Arduino until one contains `text`."""
    end = time.time() + timeout
    while time.time() < end:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print(f"  [arduino] {line}")
            if text in line:
                return True
    return False


def grab_fresh_frame(cap):
    """OpenCV keeps a few old frames buffered; throw them away so the photo
    shows what's on the lid right now, not what was there seconds ago."""
    for _ in range(5):
        cap.grab()
    ok, frame = cap.read()
    return frame if ok else None


# ---------------- Startup ----------------
print("Loading model...")
model = models.mobilenet_v2()
model.classifier[1] = torch.nn.Linear(model.last_channel, len(CLASSES))
model.load_state_dict(torch.load(MODEL_PATH, map_location="cpu", weights_only=True))
model.eval()
print("Model loaded.")

transform = transforms.Compose([
    transforms.Resize((224, 224)),
    transforms.ToTensor(),
])


def classify_frame(frame):
    img = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
    tensor = transform(img).unsqueeze(0)
    with torch.no_grad():
        probs = torch.softmax(model(tensor), dim=1)[0]
    pred = probs.argmax().item()
    return CLASSES[pred], probs[pred].item()


port = find_serial_port()
print(f"Connecting to Arduino on {port}...")
arduino = serial.Serial(port, BAUD, timeout=1)
# Opening the port resets the Arduino; wait for its "Ready." banner.
if not wait_for_line(arduino, "Ready", timeout=5):
    print("  (no Ready message - continuing anyway)")
arduino.reset_input_buffer()

chip_path = find_gpio_chip()
gpio = gpiod.request_lines(
    chip_path,
    consumer="waste-sorter",
    config={IR_SENSOR_PIN: gpiod.LineSettings(direction=Direction.INPUT,
                                               active_low=IR_ACTIVE_LOW)},
)

cap = cv2.VideoCapture(CAMERA_INDEX)
if not cap.isOpened():
    sys.exit(f"ERROR: Cannot open camera {CAMERA_INDEX}. Try CAMERA_INDEX = 1.")

print("System ready. Waiting for waste...")

# ---------------- Main loop ----------------
try:
    while True:
        if gpio.get_value(IR_SENSOR_PIN) != Value.ACTIVE:
            time.sleep(0.05)
            continue

        print("Object detected!")
        time.sleep(CAPTURE_DELAY)

        frame = grab_fresh_frame(cap)
        if frame is None:
            print("  Camera capture failed")
            continue

        label, confidence = classify_frame(frame)
        print(f"  -> {label} ({confidence * 100:.1f}%)")
        if confidence < MIN_CONFIDENCE:
            print(f"  -> Not sure, sending to trash")
            label = "trash"

        arduino.reset_input_buffer()
        arduino.write(ARDUINO_COMMAND[label])
        print(f"  -> Sent '{label}' to Arduino, waiting for it to finish...")

        if not wait_for_line(arduino, "Done", timeout=ARDUINO_TIMEOUT):
            print("  WARNING: Arduino never said Done. Check wiring/power.")

        time.sleep(COOLDOWN)

except KeyboardInterrupt:
    print("Shutting down...")

finally:
    cap.release()
    arduino.close()
    gpio.release()
    print("Cleanup complete.")
