#pragma once

#ifdef PIN_VIBRATION

#include <Arduino.h>

/*
 * Vibration motor control class
 *
 * Provides vibration feedback for events like new messages and new contacts
 * Features:
 * - 1-second vibration pulse
 * - 5-second nag timeout (cooldown between vibrations)
 * - Non-blocking operation
 */

#ifndef VIBRATION_TIMEOUT
#define VIBRATION_TIMEOUT 5000 // 5 seconds default
#endif

class GenericVibration {
public:
  void begin();       // set up vibration pin
  void trigger(unsigned long pulse_millis = VIBRATION_TIMEOUT);  // trigger vibration if cooldown has passed   // trigger vibration pulse
  void loop();        // non-blocking timer handling
  bool isVibrating(); // returns true if currently vibrating
  void stop();        // stop vibration immediately

private:
  unsigned long _started_at;
  unsigned long _pulse_millis;
};

#endif // ifdef PIN_VIBRATION
