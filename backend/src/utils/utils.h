#include <cpprest/http_client.h>
#include <functional>
#include <string>

#include "file-logger.h"

void log_service_start(FileLogger, web::uri, std::string db_path,
                       std::string log_path);
void log_service_stop(FileLogger);
void log_service_start_failed(FileLogger, std::exception&);

void freeze_thread_until_cin_closed();

std::string get_now_datetime_iso();
std::string ms_since(std::chrono::time_point<std::chrono::steady_clock> start);

std::string query(web::http::http_request req, std::string key);
int query(web::http::http_request req, std::string key, int defaultVal);