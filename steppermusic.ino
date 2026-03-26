/*
  steppermusic.ino
  Arduino Nano + two dual-channel motor drivers + up to 4 bipolar stepper motors
  Receives MIDI note numbers via Serial and drives steppers in real-time

  Wiring: (Example with L298N or TB6612 style drivers)
  Driver #1 (Motor A - channels 1,2): D2,D3,D4,D5
  Driver #2 (Motor B - channels 3,4): D8,D9,D10,D11
  Serial speed: 115200 baud

  Serial protocol: Send byte 0x90 + MidiNote (0-87 for piano keys) + 0xFF when done
*/

// Motor control pins (2 motors only)
const uint8_t motorPins[2][4] = {
  {2, 3, 4, 5},      // Motor 1 (Driver 1)
  {8, 9, 10, 11}     // Motor 2 (Driver 2)
};

// 4-step full-step sequence for bipolar stepper
const uint8_t stepTable[4][4] = {
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

// Motor state: 0=off, 1-4=step index
uint8_t motorStates[2] = {0, 0};
uint8_t motorStepIndices[2] = {0, 0};

// Target frequencies for each motor (Hz)
float motorFreqs[2] = {0.0f, 0.0f};
unsigned long motorLastStep[2] = {0, 0};

// MIDI note to frequency conversion
// MIDI note 0 = C-1 (8.1758 Hz)
// MIDI note 12 = C0 (16.3516 Hz)
// MIDI note 69 = A4 (440 Hz)
// MIDI note 87 = B8 (7040 Hz)
float midiNoteToFreq(uint8_t midiNote) {
  // f = 440 * 2^((n-69)/12)
  return 440.0f * pow(2.0f, (midiNote - 69) / 12.0f);
}

void setMotorStep(uint8_t motorNum, uint8_t stepIndex) {
  if (motorNum >= 2) return;
  const uint8_t* pins = motorPins[motorNum];
  digitalWrite(pins[0], stepTable[stepIndex][0]);
  digitalWrite(pins[1], stepTable[stepIndex][1]);
  digitalWrite(pins[2], stepTable[stepIndex][2]);
  digitalWrite(pins[3], stepTable[stepIndex][3]);
}

void motorOff(uint8_t motorNum) {
  if (motorNum >= 2) return;
  const uint8_t* pins = motorPins[motorNum];
  digitalWrite(pins[0], LOW);
  digitalWrite(pins[1], LOW);
  digitalWrite(pins[2], LOW);
  digitalWrite(pins[3], LOW);
}

void playNote(uint8_t motorNum, uint8_t midiNote) {
  if (motorNum >= 2 || midiNote > 87) return;
  motorFreqs[motorNum] = midiNoteToFreq(midiNote);
  motorStates[motorNum] = 1;  // Active
  motorLastStep[motorNum] = micros();
}

void stopNote(uint8_t motorNum) {
  if (motorNum >= 2) return;
  motorStates[motorNum] = 0;  // Off
  motorFreqs[motorNum] = 0.0f;
  motorOff(motorNum);
}

void setup() {
  Serial.begin(115200);
  
  for (uint8_t m = 0; m < 2; m++) {
    for (uint8_t i = 0; i < 4; i++) {
      pinMode(motorPins[m][i], OUTPUT);
      motorOff(m);
    }
  }
}

void loop() {
  unsigned long nowUs = micros();

  // Update all active motors (2 motors only)
  for (uint8_t m = 0; m < 2; m++) {
    if (motorStates[m] == 1 && motorFreqs[m] > 0.0f) {
      unsigned long interval = (unsigned long)(1000000.0f / motorFreqs[m]);
      if ((unsigned long)(nowUs - motorLastStep[m]) >= interval) {
        motorStepIndices[m] = (motorStepIndices[m] + 1) & 0x03;
        setMotorStep(m, motorStepIndices[m]);
        motorLastStep[m] += interval;
      }
    }
  }

  // Read serial commands
  // Format: 0x90 + midiNote, or 0x80 + motorNum to stop
  if (Serial.available() >= 2) {
    uint8_t cmd = Serial.read();
    uint8_t data = Serial.read();

    if (cmd == 0x90) {
      // Play note on next available motor
      for (uint8_t m = 0; m < 2; m++) {
        if (motorStates[m] == 0) {
          playNote(m, data);
          break;
        }
      }
    } else if (cmd == 0x80) {
      // Stop note on motor
      if (data < 2) {
        stopNote(data);
      }
    } else if (cmd == 0xFF) {
      // Stop all
      for (uint8_t m = 0; m < 2; m++) {
        stopNote(m);
      }
    }
  }
}
