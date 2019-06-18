#include <Arduino.h>
#include <sys/time.h>
#include <chrono>

#define METEOS_DEBUG_LOG 1

#if METEOS_DEBUG_LOG
#define METEOS_LOG log
#define METEOS_LOG_LN log_ln
#define METEOS_SCOPED_LOGGER(args) \
  ScopedLogger _ { args }
#else
#define METEOS_LOG
#define METEOS_LOG_LN
#define METEOS_SCOPED_LOGGER
#endif

std::chrono::milliseconds time();

void log(String str, bool to_display = false);
void log(uint64_t num);
void log_ln(String str, bool to_display = false);

class ScopedLogger {
 public:
  ScopedLogger(String msg_, bool to_display = false)
      : msg{msg_}, to_display_{to_display} {
    start = time();
    log_ln(msg + "...", to_display_);
  }

  ~ScopedLogger() {
    log_ln(msg + "...done in " + String((int)(time() - start).count()) + "ms",
           to_display_);
  }

 private:
  std::chrono::milliseconds start;
  String msg;
  bool to_display_;
};