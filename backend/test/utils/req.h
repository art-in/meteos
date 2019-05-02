#include <cpprest/http_client.h>
#include <cpprest/json.h>

web::json::value request_json(
    web::http::client::http_client& client, web::http::method mtd,
    std::string path = "", web::json::value const& jvalue = web::json::value());

std::string request_string(web::http::client::http_client& client,
                           web::http::method mtd, std::string path = "",
                           web::json::value const& jvalue = web::json::value());