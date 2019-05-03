#include <cpprest/http_client.h>
#include <cpprest/json.h>

web::http::http_response get(web::http::client::http_client& client,
                             std::string path);

web::http::http_response post(web::http::client::http_client& client,
                              std::string path, std::string const body,
                              std::string content_type);

web::http::http_response post(web::http::client::http_client& client,
                              std::string path, web::json::value const jvalue);

web::http::http_response del(web::http::client::http_client& client,
                             std::string path);
