#include <soc/rtc.h>
#include <sys/time.h>

#include "radio-wifi.h"

#define BUTTON_PIN_BITMASK 0x8000  // pin 15 (2^15 in hex)

class Sleep {
 public:
  esp_sleep_wakeup_cause_t on_wakeup();
  void print_wakeup_reason(esp_sleep_wakeup_cause_t);
  void plan_deep_sleep();

  static bool is_wakeup_goal_make_sample();

 private:
  void deep_sleep(int us);
};
