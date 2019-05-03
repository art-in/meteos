#include <cpprest/http_client.h>
#include <cpprest/json.h>

namespace json = web::json;

using utility::string_t;
using utility::conversions::to_string_t;
using web::http::http_exception;
using web::http::http_response;
using web::http::methods;
using web::http::status_codes;
using web::http::client::http_client;

http_response get(http_client& client, std::string path) {
  return client.request(methods::GET, to_string_t(path)).get();
}

http_response post(http_client& client, std::string path, std::string body,
                   std::string content_type) {
  return client
      .request(methods::POST, to_string_t(path), to_string_t(body),
               to_string_t(content_type))
      .get();
}

http_response post(http_client& client, std::string path, json::value body) {
  return client.request(methods::POST, to_string_t(path), body).get();
}

http_response del(http_client& client, std::string path) {
  return client.request(methods::DEL, to_string_t(path)).get();
}