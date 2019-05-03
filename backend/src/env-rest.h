#pragma once

#include <cpprest/http_listener.h>

#include "env-db.h"
#include "utils/rest.h"

class EnvRest : Rest {
 public:
  EnvRest(web::uri url, EnvDb &_db);

 private:
  EnvDb db;

  void handle_samples_get(web::http::http_request req);
  void handle_samples_post(web::http::http_request req);
  void handle_samples_del(web::http::http_request req);
};
