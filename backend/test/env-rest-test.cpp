#define CATCH_CONFIG_MAIN

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <catch2/catch.hpp>

#include "utils/req.h"

namespace json = web::json;
using utility::conversions::to_string_t;
using web::http::methods;
using web::http::client::http_client;

constexpr auto ADDR = "http://localhost:8080/";

TEST_CASE("Replies with empty array if no samples") {
  http_client client(to_string_t(ADDR));
  auto response = request_string(client, methods::GET);
  REQUIRE(response == "[]");
}