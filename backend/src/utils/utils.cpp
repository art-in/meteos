#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

void freeze_thread_until_cin_closed() {
  std::string s;
  std::getline(std::cin, s);
}

std::string get_now_datetime_iso() {
  time_t now;
  time(&now);
  char buf[sizeof "XXXX-XX-XXTXX:XX:XXZ"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
  return buf;
}