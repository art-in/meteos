#pragma once

#include <cpprest/http_listener.h>

#include "env-db.h"
#include "utils/rest.h"

class EnvRest : Rest {
 public:
  EnvRest(web::uri url, EnvDb &_db);
  ~EnvRest() { close(); }

  pplx::task<void> open() { return listener.open(); }
  pplx::task<void> close() { return listener.close(); }

 private:
  web::http::experimental::listener::http_listener listener;
  EnvDb db;

  void handle_get(web::http::http_request req);
  void handle_post(web::http::http_request req);
};
