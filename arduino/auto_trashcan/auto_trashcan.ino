/*
  Automatic Trashcan - stepper turntable (3 sections) + servo lid
  Section 1 = Trash, 2 = Recycling, 3 = Compost
  Commands arrive over USB serial (9600 baud), normally from the Raspberry Pi
  (raspberry_pi/main.py). You can still type 1, 2, or 3 in the Serial Monitor
  to test without the Pi.
  The Pi waits for the "Done." line before it looks for the next item.
*/

#include <Servo.h>

// ---------- Pins ----------
const int bigStepPin = 12;
const int bigDirPin  = 13;
const int bigEnPin   = 11;   // A4988 ENABLE: LOW = enabled, HIGH = coils off
const int servoPin   = 9;    // must be a PWM-capable pin

// ---------- Motion config ----------
const int  STEPS_PER_REV  = 500;   // steps for one full turntable revolution
const int  STEP_DELAY_US  = 1000;  // half-period of the step pulse (speed)
const bool HOLD_TORQUE    = true;  // false = de-energize stepper between moves

// ---------- Servo (lid) config ----------
const int LID_CLOSED_DEG = 0;      // flat / closed
const int LID_OPEN_DEG   = 90;     // open
const unsigned long LID_OPEN_MS = 3000;  // how long the lid stays open
const int SERVO_STEP_MS = 15;            // ms per degree - larger = slower sweep

Servo lid;
int lidPos = LID_CLOSED_DEG;   // tracked so a sweep always starts where it is

int  currentSection  = 1;   // assume powered on aligned with Trash
long currentStepPos  = 0;   // absolute step position of the turntable

// Absolute step position for a section, computed from 0 each time so the
// 500/3 rounding error never accumulates.
long sectionSteps(int section) {
  return lround((section - 1) * (STEPS_PER_REV / 3.0));
}

void stepMotor(long steps, bool clockwise) {
  digitalWrite(bigDirPin, clockwise ? HIGH : LOW);
  digitalWrite(bigEnPin, LOW);              // enable driver
  delayMicroseconds(5);                     // DIR setup time

  for (long i = 0; i < steps; i++) {
    digitalWrite(bigStepPin, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(bigStepPin, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }

  if (!HOLD_TORQUE) digitalWrite(bigEnPin, HIGH);
}

void moveToSection(int target) {
  if (target == currentSection) return;

  // Shortest path: +1 = one section CW, -1 = one section CCW
  int diff = ((target - currentSection) % 3 + 3) % 3;   // 0, 1, or 2
  if (diff == 2) diff = -1;

  long targetPos = sectionSteps(target);
  long delta;

  if (diff > 0) {                       // clockwise
    delta = targetPos - currentStepPos;
    if (delta < 0) delta += STEPS_PER_REV;
    stepMotor(delta, true);
  } else {                              // counter-clockwise
    delta = currentStepPos - targetPos;
    if (delta < 0) delta += STEPS_PER_REV;
    stepMotor(delta, false);
  }

  currentStepPos = targetPos;
  currentSection = target;
}

// Moves the lid one degree at a time so it eases open/shut instead of snapping.
void sweepLidTo(int targetDeg) {
  int dir = (targetDeg > lidPos) ? 1 : -1;
  while (lidPos != targetDeg) {
    lidPos += dir;
    lid.write(lidPos);
    delay(SERVO_STEP_MS);
  }
}

void openLid() {
  sweepLidTo(LID_OPEN_DEG);
  delay(LID_OPEN_MS);
}

void closeLid() {
  sweepLidTo(LID_CLOSED_DEG);
}

void setup() {
  pinMode(bigStepPin, OUTPUT);
  pinMode(bigDirPin,  OUTPUT);
  pinMode(bigEnPin,   OUTPUT);
  digitalWrite(bigEnPin, HOLD_TORQUE ? LOW : HIGH);

  lid.attach(servoPin);
  lid.write(LID_CLOSED_DEG);  // snap to a known position on boot
  lidPos = LID_CLOSED_DEG;
  delay(500);

  Serial.begin(9600);
  Serial.println(F("Ready. Enter 1 = Trash, 2 = Recycling, 3 = Compost"));
}

void loop() {
  if (!Serial.available()) return;

  char c = Serial.read();
  if (c < '1' || c > '3') return;        // ignore newlines / junk

  int target = c - '0';
  Serial.print(F("Moving to section "));
  Serial.println(target);

  closeLid();                 // never rotate with the lid open
  moveToSection(target);
  openLid();
  closeLid();

  // Drop anything that arrived while we were busy so it can't trigger a
  // surprise second move.
  while (Serial.available()) Serial.read();

  Serial.println(F("Done."));
}
