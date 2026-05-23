#define STEP 13
#define DIR 12
#define DELAY_US 100

// temporary motor step counts for each bin position.
// these need to be calibrated with the actual motor and final bin layout.
#define STEPS_COMPOST 800
#define STEPS_RECYCLE 1600
#define STEPS_TRASH 2400

void stepMotor(int steps) {
  // send step pulses to the motor driver.
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(DELAY_US);
    digitalWrite(STEP, LOW);
    delayMicroseconds(DELAY_US);
  }
}

void setup() {
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '1') {
      Serial.println("Received: COMPOST - rotating to compost bin");
      digitalWrite(DIR, HIGH);
      stepMotor(STEPS_COMPOST);
      Serial.println("Done.");

    } else if (cmd == '2') {
      Serial.println("Received: RECYCLE - rotating to recycle bin");
      digitalWrite(DIR, LOW);
      stepMotor(STEPS_RECYCLE);
      Serial.println("Done.");

    } else if (cmd == '3') {
      Serial.println("Received: TRASH - rotating to trash bin");
      digitalWrite(DIR, HIGH);
      stepMotor(STEPS_TRASH);
      Serial.println("Done.");
    }
  }
}
