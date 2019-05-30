#include <Arduino.h>

#include "button.h"

void Button::init(int pin_,
                  std::function<void(ButtonState)> on_state_changed_) {
  pinMode(pin_, INPUT_PULLUP);
  pin = pin_;
  on_state_changed = on_state_changed_;
}

void Button::set_state(ButtonState new_state) {
  if (new_state != state_) {
    state_ = new_state;

    if (on_state_changed) {
      on_state_changed(new_state);
    }
  }
}

ButtonState Button::state() { return state_; }

ButtonState Button::read() {
  return digitalRead(pin) == 0 ? ButtonState::PRESSED : ButtonState::RELEASED;
}

void Button::update() { set_state(read()); }
