#pragma once

#include <functional>

enum class ButtonState { PRESSED, RELEASED };

class Button {
 public:
  void init(int pin, std::function<void(ButtonState)> on_state_changed);

  ButtonState state();
  void update();

 private:
  int pin;
  ButtonState state_;
  std::function<void(ButtonState)> on_state_changed;

  void set_state(ButtonState new_state);
  ButtonState read();
};
