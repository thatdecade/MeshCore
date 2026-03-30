#ifdef PIN_VIBRATION
#include "GenericVibration.h"

void GenericVibration::begin() {
  pinMode(PIN_VIBRATION, OUTPUT);
  digitalWrite(PIN_VIBRATION, LOW);
  _started_at = 0;
  _pulse_millis = VIBRATION_TIMEOUT;
}

void GenericVibration::trigger(unsigned long pulse_millis) {
  _started_at = millis();
  _pulse_millis = pulse_millis;
  digitalWrite(PIN_VIBRATION, HIGH);
}

void GenericVibration::loop() {
  if (isVibrating()) {
    digitalWrite(PIN_VIBRATION, HIGH);
    if (millis() - _started_at > _pulse_millis) {
      stop();
    }
  }
}

bool GenericVibration::isVibrating() {
  return _started_at > 0;
}

void GenericVibration::stop() {
  _started_at = 0;
  _pulse_millis = VIBRATION_TIMEOUT;
  digitalWrite(PIN_VIBRATION, LOW);
}

#endif // ifdef PIN_VIBRATION
