#pragma once

#include <chrono>
#include <functional>

enum class ButtonState { PRESSED, RELEASED };

using StateChangedCallback = std::function<void(
    ButtonState, std::chrono::milliseconds prev_state_duration)>;

class Button {
 public:
  void init(int pin, StateChangedCallback on_state_changed);

  ButtonState state();
  void update();

 private:
  int pin;
  ButtonState state_;
  StateChangedCallback on_state_changed;
  std::chrono::milliseconds state_start_time;

  void set_state(ButtonState new_state);
  ButtonState read();
};
