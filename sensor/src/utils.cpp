#include <Arduino.h>

#include "display.h"
#include "utils.h"

void log(String str, bool to_display) {
  Serial.print(str);

  // Display* disp = Display::get_instance();
  // if (to_display && disp) {
  //   (*disp).draw_log(str);
  // }
}

void log(uint64_t num) {
  uint64_t xx = num / 1000000000ULL;
  if (xx > 0) Serial.print((long)xx);
  Serial.print((long)(num - xx * 1000000000));
}

void log_ln(String str, bool to_display) { log(str + "\n", to_display); }

void log(long long num) {
  long long xx = num / 1000000000ULL;
  if (xx > 0) Serial.print((long)xx);
  Serial.println((long)(num - xx * 1000000000));
}

std::chrono::milliseconds time() {
  const auto p1 = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      p1.time_since_epoch());
}