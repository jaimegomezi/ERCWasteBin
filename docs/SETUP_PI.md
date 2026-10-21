# Raspberry Pi Setup (first time)

This gets a brand-new Pi running `raspberry_pi/main.py`, talking to the Arduino,
and starting automatically on boot.

## 1. Flash the SD card

1. On your laptop, install **Raspberry Pi Imager** from raspberrypi.com/software.
2. Choose your Pi model -> **Raspberry Pi OS (64-bit)** -> your SD card.
   It must be the 64-bit version, or PyTorch won't install.
3. When asked about OS customization, choose **Edit Settings**:
   - Hostname: `wastepi`
   - Username / password: pick something and write it down
   - Wi-Fi name and password
   - **Services** tab: turn on **SSH** (use password authentication)
4. Write the card, put it in the Pi, power it on, and wait ~2 minutes.

## 2. Log in

With a monitor and keyboard, just open the Terminal app. Otherwise, from a
laptop on the same Wi-Fi:

```bash
ssh yourusername@wastepi.local
```

School Wi-Fi often blocks this. A phone hotspot or home network is easier.

## 3. Wire it up (Pi powered OFF)

**IR sensor -> Pi 40-pin header**

| IR sensor | Pi pin                 |
|-----------|------------------------|
| VCC       | Pin 1 (3.3V)           |
| GND       | Pin 6 (GND)            |
| OUT       | Pin 11 (GPIO 17)       |

Use **3.3V, not 5V**. A sensor powered from 5V sends 5V into the Pi's input
pin, which can damage it.

**Camera:** USB webcam into any USB port.

**Arduino:** USB cable from the Arduino to the Pi. This carries the commands
and powers the Arduino's logic. The stepper and servo still need their own
power supply, as they do now.

## 4. Install the project

```bash
sudo apt update && sudo apt full-upgrade -y
sudo apt install -y git python3-venv python3-dev build-essential
sudo reboot
```

Log back in, then:

```bash
git clone https://github.com/Htetty/ERC.git
cd ERC
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

The `pip install` can take 10-20 minutes on a Pi. That's normal.

## 5. Test the pieces one at a time

Run these from the `ERC` folder with the venv active (`source .venv/bin/activate`).

**Model works:**
```bash
python3 scripts/sample_img_test.py
```

**Arduino is seen:** plug it in and run `ls /dev/ttyACM* /dev/ttyUSB*`.
You should see one device.

**Camera is seen:** `ls /dev/video*` should list devices. If `main.py` says it
can't open the camera, change `CAMERA_INDEX` at the top of `main.py` to `1`.

## 6. Run it

```bash
python3 raspberry_pi/main.py
```

You should see `Ready.` from the Arduino, then `System ready. Waiting for waste...`.
Put something in front of the IR sensor and watch it classify and sort.
Press `Ctrl+C` to stop.

**If it says "Object detected!" over and over with nothing there,** set
`IR_ACTIVE_LOW = False` near the top of `main.py`. If it never detects
anything, check the small potentiometer on the IR module (it sets range) and
that its LED lights up when you hold your hand in front of it.

## 7. Start automatically on boot

Create a service file:

```bash
sudo nano /etc/systemd/system/waste-sorter.service
```

Paste this, replacing `yourusername` in all three places:

```ini
[Unit]
Description=ERC waste sorter
After=multi-user.target

[Service]
User=yourusername
WorkingDirectory=/home/yourusername/ERC
ExecStart=/home/yourusername/ERC/.venv/bin/python raspberry_pi/main.py
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Save (`Ctrl+O`, `Enter`, `Ctrl+X`), then:

```bash
sudo systemctl enable --now waste-sorter
```

Useful commands:

- See what it's printing: `journalctl -u waste-sorter -f`
- Stop it (e.g. to run it by hand): `sudo systemctl stop waste-sorter`
- Start it again: `sudo systemctl start waste-sorter`

## Updating code later

```bash
cd ~/ERC && git pull
sudo systemctl restart waste-sorter
```
