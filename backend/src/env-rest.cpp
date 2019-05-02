#include "env-rest.h"
#include "env-db.h"
#include "utils/utils.h"

namespace json = web::json;
using std::placeholders::_1;
using utility::conversions::to_string_t;
using web::http::http_request;
using web::http::methods;
using web::http::status_codes;

EnvRest::EnvRest(web::uri url, EnvDb &_db) : listener{url}, db{_db} {
  listener.support(methods::GET, std::bind(&EnvRest::handle_get, this, _1));
  listener.support(methods::POST, std::bind(&EnvRest::handle_post, this, _1));
  open().wait();
}

void EnvRest::handle_get(http_request req) {
  handle(req, [&] {
    auto samples = db.get_samples();
    auto resp = json::value::array();

    for (int i = 0; i < samples.size(); i++) {
      auto sample = samples[i];
      auto datetime = to_string_t(sample.datetime);

      auto sample_obj = json::value::object();

      sample_obj[U("u")] = json::value::string(datetime);
      sample_obj[U("t")] = sample.temperature;
      sample_obj[U("h")] = sample.humidity;
      sample_obj[U("p")] = sample.pressure;
      sample_obj[U("c")] = sample.co2;

      resp[i] = sample_obj;
    }

    req.reply(status_codes::OK, resp);
  });
};

void EnvRest::handle_post(http_request req) {
  handle(req, [&] {
    json::value obj = req.extract_json().get();

    auto datetime = get_now_datetime_iso();
    auto temperature = obj[U("t")].as_double();
    auto humidity = obj[U("h")].as_double();
    auto pressure = obj[U("p")].as_double();
    auto co2 = obj[U("c")].as_double();

    db.add_sample({datetime, temperature, humidity, pressure, co2});

    req.reply(status_codes::OK);
  });
};
