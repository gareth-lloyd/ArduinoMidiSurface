#ifndef MidiSurface_h
#define MidiSurface_h

#include "Arduino.h"
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

#define MOVING_AVG_LENGTH 5
#define DEBOUNCE_CYCLES 50

#define PROGRAM_CHANGE 'p'
#define CONTROL_CHANGE 'c'
#define INCREMENT_CC 'i'
#define DECREMENT_CC 'd'
#define MIN_CC 0
#define MAX_CC 127

typedef struct {
  int pin;
  int totalPreviousReadings;
  int currentReading;
  int cc;
} AnalogControl;

typedef struct {
  int pin;
  bool lastReading;
  int debounceCyclesRemaining;
  char outputType;
  int cc;
  int value;
} DigitalControl;

int scaleToCCRange(int reading, int CCMin, int CCMax);

void scanAnalogControl(MidiInterface& midi, AnalogControl& control);

void scanDigitalControl(MidiInterface& midi, DigitalControl& control);

void performDigitalControl(MidiInterface& midi, DigitalControl& control);
#endif
