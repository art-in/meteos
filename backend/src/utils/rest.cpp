#include <cpprest/http_client.h>
#include <algorithm>
#include <exception>
#include <string>

#include "rest.h"

using std::placeholders::_1;
using utility::string_t;
using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;
using web::http::http_request;
using web::http::methods;
using web::http::status_codes;

Rest::Rest(web::uri url, FileLogger _logger) : listener{url}, logger{_logger} {
  listener.support(std::bind(&Rest::handle_request, this, _1));
  open().wait();
}

void Rest::reg_handler(web::http::method method, std::string path,
                       std::function<void(http_request, string_t)> handler) {
  handlers.push_back({method, to_string_t(path), handler});
}

void Rest::handle_request(http_request req) {
  string_t body = req.extract_string().get();

  std::string log_record = to_utf8string(req.remote_address()) + " - " +
                           to_utf8string(req.method()) + " " +
                           to_utf8string(req.relative_uri().to_string()) +
                           " - " + to_utf8string(body);

  try {
    auto filter = [&req](RequestHandler &h) {
      return h.method == req.method() && h.path == req.relative_uri().path();
    };

    auto handler_it = std::find_if(handlers.begin(), handlers.end(), filter);

    if (handler_it != handlers.end()) {
      (*handler_it).fn(req, body);
      logger.log(LogLevel::INFO, log_record);
    } else {
      req.reply(status_codes::NotFound);
      logger.log(LogLevel::WARN, log_record + " - NOT FOUND");
    }
  } catch (std::exception &e) {
    req.reply(status_codes::InternalError);
    logger.log(LogLevel::ERROR, log_record + " - " + e.what());
  }
}