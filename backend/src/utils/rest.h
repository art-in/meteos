#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <functional>
#include <string>

#include "file-logger.h"

struct RequestHandler {
  web::http::method method;
  utility::string_t path;
  std::function<void(web::http::http_request, utility::string_t body)> fn;
};

class Rest {
 protected:
  Rest(web::uri url, FileLogger);
  ~Rest() { close(); }

  pplx::task<void> open() { return listener.open(); }
  pplx::task<void> close() { return listener.close(); }

  void reg_handler(
      web::http::method method, std::string path,
      std::function<void(web::http::http_request, utility::string_t)> handler);

 private:
  FileLogger logger;
  web::http::experimental::listener::http_listener listener;
  std::vector<RequestHandler> handlers;

  void handle_request(web::http::http_request req);
};
