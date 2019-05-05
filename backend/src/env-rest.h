#pragma once

#include <cpprest/http_listener.h>

#include "env-db.h"
#include "utils/file-logger.h"
#include "utils/rest.h"

class EnvRest : Rest {
 public:
  EnvRest(web::uri, EnvDb &, FileLogger);

 private:
  EnvDb db;

  void handle_samples_get(web::http::http_request);
  void handle_samples_post(web::http::http_request, utility::string_t body);
  void handle_samples_del(web::http::http_request);
};
