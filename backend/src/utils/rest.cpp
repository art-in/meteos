#include <algorithm>
#include <exception>
#include <string>

#include "rest.h"

using std::placeholders::_1;
using utility::conversions::to_string_t;
using web::http::http_request;
using web::http::method;
using web::http::status_codes;

Rest::Rest(web::uri url) : listener{url} {
  listener.support(std::bind(&Rest::handle_request, this, _1));
  open().wait();
}

void Rest::reg_handler(web::http::method method, std::string path,
                       std::function<void(http_request)> handler) {
  handlers.push_back({method, to_string_t(path), handler});
}

void Rest::handle_request(http_request req) {
  try {
    auto filter = [&req](RequestHandler &h) {
      return h.method == req.method() && h.path == req.relative_uri().path();
    };

    auto handler_it = std::find_if(handlers.begin(), handlers.end(), filter);

    if (handler_it != handlers.end()) {
      (*handler_it).fn(req);
    } else {
      req.reply(status_codes::NotFound);
    }
  } catch (std::exception &e) {
    // TODO: log to file
    ucerr << "Failed to handle request: " << e.what() << std::endl;
    ucerr << req.to_string() << std::endl;

    std::string resp =
        std::string{e.what()} + " " + req.extract_utf8string().get();
    req.reply(status_codes::InternalError, resp);
  }
}