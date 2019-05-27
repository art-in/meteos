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

uint64_t get_epoch_time_us() {
  timeval time;
  timezone tz;
  gettimeofday(&time, &tz);
  return static_cast<uint64_t>(time.tv_sec) * uS_TO_S_FACTOR +
         static_cast<uint64_t>(time.tv_usec);
}