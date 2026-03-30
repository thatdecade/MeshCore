#include "MomentaryButton.h"

#define MULTI_CLICK_WINDOW_MS  280

MomentaryButton::MomentaryButton(int8_t pin, int long_press_millis, bool reverse, bool pulldownup, bool multiclick, int debounce_millis) { 
  _pin = pin;
  _reverse = reverse;
  _pull = pulldownup;
  down_at = 0; 
  cancel = 0;
  _long_millis = long_press_millis;
  _threshold = 0;
  _click_count = 0;
  _last_click_time = 0;
  _multi_click_window = multiclick ? MULTI_CLICK_WINDOW_MS : 0;
  _pending_click = false;
  _raw_level = _reverse ? HIGH : LOW;
  _stable_level = _raw_level;
  _last_raw_change_time = 0;
  _debounce_millis = debounce_millis;
}

MomentaryButton::MomentaryButton(int8_t pin, int long_press_millis, int analog_threshold, int debounce_millis) {
  _pin = pin;
  _reverse = false;
  _pull = false;
  down_at = 0;
  cancel = 0;
  _long_millis = long_press_millis;
  _threshold = analog_threshold;
  _click_count = 0;
  _last_click_time = 0;
  _multi_click_window = MULTI_CLICK_WINDOW_MS;
  _pending_click = false;
  _raw_level = LOW;
  _stable_level = LOW;
  _last_raw_change_time = 0;
  _debounce_millis = debounce_millis;
}

void MomentaryButton::begin() {
  if (_pin >= 0 && _threshold == 0) {
    pinMode(_pin, _pull ? (_reverse ? INPUT_PULLUP : INPUT_PULLDOWN) : INPUT);
  }
  if (_pin >= 0) {
      int initial_level = _threshold > 0 ? (analogRead(_pin) < _threshold) : digitalRead(_pin);
      _raw_level = initial_level;
      _stable_level = initial_level;
      _last_raw_change_time = millis();
  }
}

bool  MomentaryButton::isPressed() const {
  int btn = _threshold > 0 ? (analogRead(_pin) < _threshold) : digitalRead(_pin);
  return isPressed(btn);
}

void MomentaryButton::cancelClick() {
  cancel = 1;
  down_at = 0;
  _click_count = 0;
  _last_click_time = 0;
  _pending_click = false;
}

bool MomentaryButton::isPressed(int level) const {
  if (_threshold > 0) {
    return level;
  }
  if (_reverse) {
    return level == LOW;
  } else {
    return level != LOW;
  }
}

int MomentaryButton::check(bool repeat_click) {
  if (_pin < 0) return BUTTON_EVENT_NONE;

  int event = BUTTON_EVENT_NONE;
  int raw_level = _threshold > 0 ? (analogRead(_pin) < _threshold) : digitalRead(_pin);
  unsigned long now = millis();

  if (raw_level != _raw_level) {
      _raw_level = raw_level;
      _last_raw_change_time = now;
  }

  bool debounce_elapsed = (unsigned long)(now - _last_raw_change_time) >= (unsigned long)_debounce_millis;
  if (debounce_elapsed && _stable_level != _raw_level) {
    _stable_level = _raw_level;

    if (isPressed(_stable_level)) {
      down_at = now;
    } else {
      // button UP
      if (_long_millis > 0) {
        if (down_at > 0 && (unsigned long)(now - down_at) < (unsigned long)_long_millis) {
            _click_count++;
            _last_click_time = now;
            _pending_click = true;
        }
      } else {
          _click_count++;
          _last_click_time = now;
          _pending_click = true;
      }
      down_at = 0;
    }
  }

  if (!isPressed(_stable_level) && cancel) {
    cancel = 0;
  }

  if (_long_millis > 0 && down_at > 0 && (unsigned long)(now - down_at) >= (unsigned long)_long_millis) {
    if (_pending_click) {
      // long press during multi-click detection - cancel pending clicks
      cancelClick();
    } else {
      event = BUTTON_EVENT_LONG_PRESS;
      down_at = 0;
      _click_count = 0;
      _last_click_time = 0;
      _pending_click = false;
    }
  }
  if (down_at > 0 && repeat_click) {
    unsigned long diff = (unsigned long)(now - down_at);
    if (diff >= 700) {
      event = BUTTON_EVENT_CLICK;   // wait 700 millis before repeating the click events
    }
  }

  if (_pending_click && (unsigned long)(now - _last_click_time) >= (unsigned long)_multi_click_window) {
    if (down_at > 0) {
      // still pressed - wait for button release before processing clicks
      return event;
    }
    switch (_click_count) {
      case 1:
        event = BUTTON_EVENT_CLICK;
        break;
      case 2:
        event = BUTTON_EVENT_DOUBLE_CLICK;
        break;
      case 3:
        event = BUTTON_EVENT_TRIPLE_CLICK;
        break;
      default:
        // For 4+ clicks, treat as triple click?
        event = BUTTON_EVENT_TRIPLE_CLICK;
        break;
    }
    if (event == BUTTON_EVENT_CLICK && cancel) {
      event = BUTTON_EVENT_NONE;
    }
    _click_count = 0;
    _last_click_time = 0;
    _pending_click = false;
  }

  return event;
}
