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

SCENARIO("syncing time with backend") {
  http_client client(to_string_t(ADDR));

  WHEN("getting time") {
    auto resp = get(client, "/time");

    THEN("replies current backend time") {
      REQUIRE(resp.status_code() == status_codes::OK);
      REQUIRE_THAT(resp.extract_utf8string().get(),
                   // ISO8601
                   Matches(R"(\d{4}-\d{2}-\d{2}T\d{2}\:\d{2}\:\d{2}\.\d{3}Z)"));
    }
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

    WHEN("getting samples with 'limit' query") {
      auto resp = get(client, "/samples?limit=10");

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
      auto time = samples[1].as_object()[U("u")].as_string();
      auto from = to_utf8string(web::uri::encode_data_string(time));

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
      auto time = samples[1].as_object()[U("u")].as_string();
      auto to = to_utf8string(web::uri::encode_data_string(time));

      resp = get(client, std::string{"/samples?to="} + to);

      THEN("replies with samples created before that datetime") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 2);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 1);
      }
    }

    WHEN("getting samples with 'from' and 'to' query") {
      auto time1 = samples[1].as_object()[U("u")].as_string();
      auto time2 = samples[2].as_object()[U("u")].as_string();

      auto from = to_utf8string(web::uri::encode_data_string(time1));
      auto to = to_utf8string(web::uri::encode_data_string(time2));

      resp = get(client, std::string{"/samples?from="} + from + "&to=" + to);

      THEN("replies with samples created between that datetimes") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 2);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 1);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 2);
      }
    }

    WHEN("getting samples with 'from', 'to' and 'limit' query") {
      auto time1 = samples[1].as_object()[U("u")].as_string();
      auto time2 = samples[2].as_object()[U("u")].as_string();

      auto from = to_utf8string(web::uri::encode_data_string(time1));
      auto to = to_utf8string(web::uri::encode_data_string(time2));

      resp = get(client, std::string{"/samples?from="} + from + "&to=" + to +
                             "&limit=1");

      THEN("replies with latest sample created between that datetimes") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 1);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 2);
      }
    }
  }

  GIVEN("ten samples") {
    post(client, "/samples", R"({"t":0,"h":0,"p":0,"c":0})", JSON);
    post(client, "/samples", R"({"t":1,"h":1,"p":1,"c":1})", JSON);
    post(client, "/samples", R"({"t":2,"h":2,"p":2,"c":2})", JSON);
    post(client, "/samples", R"({"t":3,"h":3,"p":3,"c":3})", JSON);
    post(client, "/samples", R"({"t":4,"h":4,"p":4,"c":4})", JSON);
    post(client, "/samples", R"({"t":5,"h":5,"p":5,"c":5})", JSON);
    post(client, "/samples", R"({"t":6,"h":6,"p":6,"c":6})", JSON);
    post(client, "/samples", R"({"t":7,"h":7,"p":7,"c":7})", JSON);
    post(client, "/samples", R"({"t":8,"h":8,"p":8,"c":8})", JSON);
    post(client, "/samples", R"({"t":9,"h":9,"p":9,"c":9})", JSON);

    WHEN("getting samples without 'limit' query") {
      auto resp = get(client, "/samples");

      THEN("replies with all samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 10);
      }
    }

    WHEN("getting samples with 'limit=-1' query") {
      auto resp = get(client, "/samples?limit=-1");

      THEN("replies with all samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 10);
      }
    }

    WHEN("getting samples with 'limit=0' query") {
      auto resp = get(client, "/samples?limit=0");

      THEN("replies with all samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 10);
      }
    }

    WHEN("getting samples with 'limit=1' query") {
      auto resp = get(client, "/samples?limit=1");

      THEN("replies with one latest sample") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 1);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=2' query") {
      auto resp = get(client, "/samples?limit=2");

      THEN("replies with earliest and latest samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 2);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=3' query") {
      auto resp = get(client, "/samples?limit=3");

      THEN("replies with samples evenly distrubuted by index") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 3);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 5);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=4' query") {
      auto resp = get(client, "/samples?limit=4");

      THEN("replies with samples evenly distributed") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 4);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 3);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 6);
        REQUIRE(samples[3].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=5' query") {
      auto resp = get(client, "/samples?limit=5");

      THEN("replies with samples evenly distributed") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 5);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 3);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 6);
        REQUIRE(samples[3].as_object()[U("t")].as_double() == 8);
        REQUIRE(samples[4].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=6' query") {
      auto resp = get(client, "/samples?limit=6");

      THEN("replies with samples evenly distributed") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 6);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 2);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 4);
        REQUIRE(samples[3].as_object()[U("t")].as_double() == 6);
        REQUIRE(samples[4].as_object()[U("t")].as_double() == 8);
        REQUIRE(samples[5].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=7' query") {
      auto resp = get(client, "/samples?limit=7");

      THEN("replies with samples evenly distributed") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 7);
        REQUIRE(samples[0].as_object()[U("t")].as_double() == 0);
        REQUIRE(samples[1].as_object()[U("t")].as_double() == 2);
        REQUIRE(samples[2].as_object()[U("t")].as_double() == 4);
        REQUIRE(samples[3].as_object()[U("t")].as_double() == 6);
        REQUIRE(samples[4].as_object()[U("t")].as_double() == 7);
        REQUIRE(samples[5].as_object()[U("t")].as_double() == 8);
        REQUIRE(samples[6].as_object()[U("t")].as_double() == 9);
      }
    }

    WHEN("getting samples with 'limit=10' query") {
      auto resp = get(client, "/samples?limit=10");

      THEN("replies with all samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 10);
      }
    }

    WHEN("getting samples with 'limit=20' query") {
      auto resp = get(client, "/samples?limit=20");

      THEN("replies with all samples") {
        auto samples = resp.extract_json().get().as_array();

        REQUIRE(samples.size() == 10);
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