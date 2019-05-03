#include <string>

/**
 * Freezes main thread until cin is closed.
 *
 * other solutions does not work:
 * - while(1), promise/wait - main function is not finished, resource classes
 *                            not cleaned up, so network port is not released.
 * - signal, sigaction - do not catch termination signals, so unable to cleanup
 *                       resource classes.
 */
void freeze_thread_until_cin_closed();

/**
 * Returns current time in ISO8601 format and UTC Zulu timezone.
 */
std::string get_now_datetime_iso();

std::string query(web::http::http_request req, std::string key);