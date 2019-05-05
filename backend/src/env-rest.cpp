#include "env-rest.h"
#include "env-db.h"
#include "utils/utils.h"

namespace json = web::json;
using std::placeholders::_1;
using std::placeholders::_2;
using utility::string_t;
using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;
using web::http::http_request;
using web::http::methods;
using web::http::status_codes;

EnvRest::EnvRest(web::uri url, EnvDb &_db, FileLogger logger)
    : Rest{url, logger}, db{_db} {
  reg_handler(methods::GET, "/samples",
              std::bind(&EnvRest::handle_samples_get, this, _1));
  reg_handler(methods::POST, "/samples",
              std::bind(&EnvRest::handle_samples_post, this, _1, _2));
  reg_handler(methods::DEL, "/samples",
              std::bind(&EnvRest::handle_samples_del, this, _1));
}

void EnvRest::handle_samples_get(http_request req) {
  std::vector<Sample> samples;

  auto from = query(req, "from");
  auto to = query(req, "to");

  samples = db.get_samples(from, to);
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
};

void EnvRest::handle_samples_post(http_request req, string_t body) {
  json::value obj = json::value::parse(body);

  auto datetime = get_now_datetime_iso();
  auto temperature = obj[U("t")].as_double();
  auto humidity = obj[U("h")].as_double();
  auto pressure = obj[U("p")].as_double();
  auto co2 = obj[U("c")].as_double();

  db.add_sample({datetime, temperature, humidity, pressure, co2});

  req.reply(status_codes::OK);
};

void EnvRest::handle_samples_del(http_request req) {
  db.delete_samples();
  req.reply(status_codes::OK);
}