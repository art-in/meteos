#include <Arduino.h>

#include <sys/time.h>

#define uS_TO_S_FACTOR 1000000
#define uS_TO_MS_FACTOR 1000

void log(String str, bool to_display = false);
void log(uint64_t num);
void log_ln(String str, bool to_display = false);

uint64_t get_epoch_time_us();