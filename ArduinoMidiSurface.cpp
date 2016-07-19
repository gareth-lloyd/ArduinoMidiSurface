#include "Arduino.h"
#include "ArduinoMidiSurface.h"

int scaleToCCRange(int reading, int CCMin, int CCMax) {
  long range = (long) (CCMax - CCMin);
  long product = reading * range;
  return (int) (product / 1024);
}

void scanAnalogControl(MidiInterface& midi, AnalogControl& control) {
  int oldValue = control.currentReading;
  control.totalPreviousReadings -= oldValue;
  control.totalPreviousReadings += analogRead(control.pin);
  control.currentReading = control.totalPreviousReadings / MOVING_AVG_LENGTH;

  if (oldValue != control.currentReading) {
    midi.sendControlChange(
      control.cc,
      scaleToCCRange(control.currentReading, MIN_CC, MAX_CC),
      SHRUTHI_OUT_CHANNEL
    );
  }
}

void scanDigitalControl(MidiInterface& midi, DigitalControl& control) {
  if (control.debounceCyclesRemaining) {
    control.debounceCyclesRemaining--;
    return;
  }
  int reading = digitalRead(control.pin);
  if (reading == HIGH && control.lastReading == LOW) {
    performDigitalControl(midi, control);
    control.debounceCyclesRemaining = DEBOUNCE_CYCLES;
  }
  control.lastReading = reading;
}

void performDigitalControl(MidiInterface& midi, DigitalControl& control) {
  switch (control.outputType) {
    case CONTROL_CHANGE:
      midi.sendControlChange(control.cc, control.value, SHRUTHI_OUT_CHANNEL);
      break;
    case PROGRAM_CHANGE:
      midi.sendProgramChange(control.value, SHRUTHI_OUT_CHANNEL);
      break;
    case DECREMENT_CC:
      if (control.value >= MIN_CC) {
        midi.sendControlChange(control.cc, --control.value, SHRUTHI_OUT_CHANNEL);
      }
      break;
    case INCREMENT_CC:
      if (control.value < MAX_CC) {
        midi.sendControlChange(control.cc, ++control.value, SHRUTHI_OUT_CHANNEL);
      }
      break;
  }
}
