#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <functional>
#include <string>

struct RequestHandler {
  web::http::method method;
  utility::string_t path;
  std::function<void(web::http::http_request)> fn;
};

class Rest {
 protected:
  Rest(web::uri url);
  ~Rest() { close(); }

  pplx::task<void> open() { return listener.open(); }
  pplx::task<void> close() { return listener.close(); }

  void reg_handler(web::http::method method, std::string path,
                   std::function<void(web::http::http_request)> handler);

 private:
  web::http::experimental::listener::http_listener listener;
  std::vector<RequestHandler> handlers;

  void handle_request(web::http::http_request req);
};
