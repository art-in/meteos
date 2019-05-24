#pragma once

enum class ButtonState { ON, OFF };

class Button {
  int pin;
  ButtonState state_;

  void set_state(ButtonState new_state);
  ButtonState read();

 public:
  Button(int pin);
  ButtonState state();
  void update();

  void (*on_state_changed)(ButtonState);
};
