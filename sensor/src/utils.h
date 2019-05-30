#include <Arduino.h>
#include <chrono>

#include <sys/time.h>

#define uS_TO_S_FACTOR 1000000
#define uS_TO_MS_FACTOR 1000

// TODO: log conditionaly with macros
void log(String str, bool to_display = false);
void log(uint64_t num);
void log_ln(String str, bool to_display = false);

std::chrono::milliseconds time();