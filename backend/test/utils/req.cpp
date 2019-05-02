#include <cpprest/http_client.h>
#include <cpprest/json.h>

namespace json = web::json;
using utility::string_t;
using utility::conversions::to_string_t;
using web::http::http_exception;
using web::http::http_response;
using web::http::method;
using web::http::methods;
using web::http::status_codes;
using web::http::client::http_client;

pplx::task<http_response> make_request_task(http_client& client, method mtd,
                                            std::string path,
                                            json::value const& jvalue) {
  string_t p = to_string_t(path);

  pplx::task<http_response> request_task =
      (mtd == methods::GET || mtd == methods::HEAD)
          ? client.request(mtd, p)
          : client.request(mtd, p, jvalue);

  return request_task.then([](http_response response) {
    if (response.status_code() != status_codes::OK) {
      throw std::runtime_error("Response code is not OK");
    }

    return response;
  });
}

json::value request_json(http_client& client, method mtd, std::string path = "",
                         json::value const& jvalue = json::value()) {
  return make_request_task(client, mtd, path, jvalue)
      .then([](http_response resp) { return resp.extract_json(); })
      .get();
}

std::string request_string(http_client& client, method mtd,
                           std::string path = "",
                           json::value const& jvalue = json::value()) {
  return make_request_task(client, mtd, path, jvalue)
      .then([](http_response resp) { return resp.extract_utf8string(); })
      .get();
}