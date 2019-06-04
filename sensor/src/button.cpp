#include <Arduino.h>

#include "button.h"
#include "utils.h"

void Button::init(int pin_, StateChangedCallback on_state_changed_) {
  pinMode(pin_, INPUT_PULLUP);
  pin = pin_;
  on_state_changed = on_state_changed_;

  state_ = ButtonState::RELEASED;
  state_start_time = time();
}

void Button::set_state(ButtonState new_state) {
  if (new_state != state_) {
    auto prev_state_duration = time() - state_start_time;

    state_ = new_state;
    state_start_time = time();

    if (on_state_changed) {
      on_state_changed(new_state, prev_state_duration);
    }
  }
}

ButtonState Button::state() { return state_; }

ButtonState Button::read() {
  return digitalRead(pin) == 0 ? ButtonState::PRESSED : ButtonState::RELEASED;
}

void Button::update() { set_state(read()); }
