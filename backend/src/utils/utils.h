#include <cpprest/http_client.h>
#include <functional>
#include <string>

#include "file-logger.h"

void log_service_start(FileLogger, web::uri, std::string db_path,
                       std::string log_path);
void log_service_stop(FileLogger);
void log_service_start_failed(FileLogger, std::exception&);

// Freezes main thread until cin is closed.
// Other solutions does not work:
// - while(1), promise/wait - main function is not finished, resource classes
//                            not cleaned up, so network port is not released.
// - signal, sigaction - do not catch termination signals, so unable to cleanup
//                        resource classes.
void freeze_thread_until_cin_closed();

// Returns current time in ISO8601 format and UTC Zulu timezone.
std::string get_now_datetime_iso();

std::string query(web::http::http_request req, std::string key);
int query(web::http::http_request req, std::string key, int defaultVal);

// Visits N items from target array.
// - ensures visited items are evenly distributed along the target
// - ensures first and last items visited
// - ensures sequence of visited items preserved
template <class T>
void distribute_evenly(int n, std::vector<T> target,
                       std::function<void(T&)> visit);