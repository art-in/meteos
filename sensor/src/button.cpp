#include <Arduino.h>

#include "button.h"

Button::Button(int p) {
  pinMode(p, INPUT_PULLUP);
  pin = p;
}

void Button::set_state(ButtonState new_state) {
  if (new_state != state_) {
    state_ = new_state;

    if (on_state_changed != nullptr) {
      on_state_changed(new_state);
    }
  }
}

ButtonState Button::state() { return state_; }

ButtonState Button::read() {
  return digitalRead(pin) == 0 ? ButtonState::ON : ButtonState::OFF;
}

void Button::update() { set_state(read()); }
