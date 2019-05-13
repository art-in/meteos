#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <catch2/catch.hpp>

#include "utils/req.h"

namespace json = web::json;

using Catch::Matchers::Matches;
using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;
using web::http::http_response;
using web::http::methods;
using web::http::status_codes;
using web::http::client::http_client;

constexpr auto ADDR = "http://localhost:3000/";
constexpr auto JSON = "application/json";

SCENARIO("getting from unknown endpoint") {
  http_client client(to_string_t(ADDR));
  auto resp = get(client, "/unknown");

  THEN("replies with status code 'NOT FOUND'") {
    REQUIRE(resp.status_code() == status_codes::NotFound);
  }
}

SCENARIO("getting samples") {
  http_client client(to_string_t(ADDR));
  del(client, "/samples");

  GIVEN("no samples") {
    WHEN("getting samples") {
      auto resp = get(client, "/samples");

      THEN("replies with empty array") {
        REQUIRE(resp.status_code() == status_codes::OK);
        REQUIRE(resp.extract_utf8string().get() == "[]");
      }
    }
  }

  GIVEN("one sample") {
    auto resp = post(client, "/samples", R"({"t":1,"h":2,"p":3,"c":4})", JSON);
    REQUIRE(resp.status_code() == status_codes::OK);

    WHEN("getting samples") {
      auto resp = get(client, "/samples");
      REQUIRE(resp.status_code() == status_codes::OK);
      auto resp_json = resp.extract_json().get();

      THEN("replies with one sample") {
        REQUIRE(resp_json.as_array().size() == 1);
      }

      THEN("replies with correct sample data") {
        auto samples = resp_json.as_array();
        auto sample = samples[0].as_object();

        REQUIRE(sample.size() == 5);
        REQUIRE(sample[U("t")].as_double() == 1);
        REQUIRE(sample[U("h")].as_double() == 2);
        REQUIRE(sample[U("p")].as_double() == 3);
        REQUIRE(sample[U("c")].as_double() == 4);
        REQUIRE_THAT(
            to_utf8string(sample[U("u")].as_string()),
            // ISO8601
            Matches(R"(\d{4}-\d{2}-\d{2}T\d{2}\:\d{2}\:\d{2}\.\d{3}Z)"));
      }
    }
  }

  GIVEN("several samples") {
    post(client, "/samples", R"({"t":0,"h":0,"p":0,"c":0})", JSON);
    post(client, "/samples", R"({"t":1,"h":1,"p":1,"c":1})", JSON);
    post(client, "/samples", R"({"t":2,"h":2,"p":2,"c":2})", JSON);
    post(client, "/samples", R"({"t":3,"h":3,"p":3,"c":3})", JSON);

    auto resp = get(client, "/samples");

    auto samples = resp.extract_json().get().as_array();
    REQUIRE(samples.size() == 4);

    WHEN("getting samples with 'from' query") {
      std::string from =
          to_utf8string(samples[1].as_object()[U("u")].as_string());
      resp = get(client, std::string{"/samples?from="} + from);

      THEN("replies with samples created after that datetime") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 3);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 1);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 2);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 3);
      }
    }

    WHEN("getting samples with 'to' query") {
      std::string to =
          to_utf8string(samples[1].as_object()[U("u")].as_string());
      resp = get(client, std::string{"/samples?to="} + to);

      THEN("replies with samples created before that datetime") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 2);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 1);
      }
    }

    WHEN("getting samples with 'from' and 'to' query") {
      std::string from =
          to_utf8string(samples[1].as_object()[U("u")].as_string());
      std::string to =
          to_utf8string(samples[2].as_object()[U("u")].as_string());
      resp = get(client, std::string{"/samples?from="} + from + "&to=" + to);

      THEN("replies with samples created between that datetime") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 2);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 1);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 2);
      }
    }
  }
}

SCENARIO("posting samples") {
  http_client client(to_string_t(ADDR));
  del(client, "/samples");

  WHEN("posting invalid json") {
    auto resp = post(client, "/samples", "{invalid}", JSON);

    THEN("replies with status code 'INTERNAL ERROR'") {
      REQUIRE(resp.status_code() == status_codes::InternalError);
    }
  }

  WHEN("posting valid json without content type header") {
    auto resp = post(client, "/samples", R"({"t":1,"h":2,"p":3,"c":4})", "");

    THEN("replies with status code 'INTERNAL ERROR'") {
      REQUIRE(resp.status_code() == status_codes::InternalError);
    }
  }

  WHEN("posting sample without co2 value") {
    auto resp = post(client, "/samples", R"({"t":1,"h":1,"p":1})", JSON);

    THEN("replies with status code 'INTERNAL ERROR'") {
      REQUIRE(resp.status_code() == status_codes::InternalError);
    }
  }

  WHEN("posting sample with big numbers") {
    auto resp = post(client, "/samples",
                     R"({"t":10000,"h":20000,"p":30000,"c":40000})", JSON);

    THEN("replies with status code 'OK'") {
      REQUIRE(resp.status_code() == status_codes::OK);
    }
  }

  WHEN("posting sample with fractional numbers") {
    auto resp = post(client, "/samples",
                     R"({"t":27.1,"h":40.12,"p":748.123,"c":1100.1234})", JSON);

    THEN("replies with status code 'OK'") {
      REQUIRE(resp.status_code() == status_codes::OK);
    }
  }
}

SCENARIO("deleting samples") {
  http_client client(to_string_t(ADDR));
  del(client, "/samples");

  GIVEN("several samples") {
    post(client, "/samples", R"({"t":1,"h":1,"p":1,"c":1})", JSON);
    post(client, "/samples", R"({"c":1,"p":1,"h":1,"t":1})", JSON);
    post(client, "/samples", R"({"p":1,"t":1,"c":1,"h":1})", JSON);
    post(client, "/samples", R"({"t":1,"h":1,"p":1,"c":1})", JSON);

    auto resp = get(client, "/samples");

    auto samples = resp.extract_json().get().as_array();
    REQUIRE(samples.size() == 4);

    WHEN("deleting samples") {
      auto resp = del(client, "/samples");
      REQUIRE(resp.status_code() == status_codes::OK);

      THEN("replies with no sample") {
        auto resp = get(client, "/samples");

        REQUIRE(resp.status_code() == status_codes::OK);
        REQUIRE(resp.extract_utf8string().get() == "[]");
      }
    }
  }
}