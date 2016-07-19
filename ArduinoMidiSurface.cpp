#include "ArduinoMidiSurface.h"

#include <Arduino.h>

#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MidiSurface::MidiSurface(
  midi::MidiInterface<HardwareSerial>* midiI, int outChannel
) {
  midiInterface = midiI;
  outChannel = outChannel;
  midiInterface->begin(outChannel);
}

int MidiSurface::scaleToCCRange(int reading, int CCMin, int CCMax) {
  long range = (long) (CCMax - CCMin);
  long product = reading * range;
  return (int) (product / 1024);
}

void MidiSurface::scanAnalogControl(
  AnalogControl& control
) {
  int oldValue = control.currentReading;
  control.totalPreviousReadings -= oldValue;
  control.totalPreviousReadings += analogRead(control.pin);
  control.currentReading = control.totalPreviousReadings / MOVING_AVG_LENGTH;

  if (oldValue != control.currentReading) {
    midiInterface->sendControlChange(
      control.cc,
      scaleToCCRange(control.currentReading, MIN_CC, MAX_CC),
      outChannel
    );
  }
}

void MidiSurface::scanDigitalControl(
  DigitalControl& control
) {
  if (control.debounceCyclesRemaining) {
    control.debounceCyclesRemaining--;
    return;
  }
  int reading = digitalRead(control.pin);
  if (reading == HIGH && control.lastReading == LOW) {
    performDigitalControl(control);
    control.debounceCyclesRemaining = DEBOUNCE_CYCLES;
  }
  control.lastReading = reading;
}

void MidiSurface::performDigitalControl(
  DigitalControl& control
) {
  switch (control.outputType) {
    case CONTROL_CHANGE:
      midiInterface->sendControlChange(
        control.cc, control.value, outChannel
      );
      break;
    case PROGRAM_CHANGE:
      midiInterface->sendProgramChange(control.value, outChannel);
      break;
    case DECREMENT_CC:
      if (control.value >= MIN_CC) {
        midiInterface->sendControlChange(
          control.cc, --control.value, outChannel
        );
      }
      break;
    case INCREMENT_CC:
      if (control.value < MAX_CC) {
        midiInterface->sendControlChange(
          control.cc, ++control.value, outChannel
        );
      }
      break;
  }
}
