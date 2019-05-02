#include <functional>

#include <cpprest/http_msg.h>

class Rest {
 protected:
  Rest() {}
  void handle(web::http::http_request req, std::function<void()> handler);
};