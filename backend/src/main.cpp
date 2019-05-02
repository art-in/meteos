#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "env-db.h"
#include "env-rest.h"
#include "utils/utils.h"

using std::filesystem::path;
using utility::conversions::to_string_t;

constexpr auto ADDR = "http://0.0.0.0:8080";
constexpr auto DB_FILE = "/data/env.db";

int main(int argc, char *argv[]) {
  std::string db_path{path(argv[0]).parent_path().u8string() + DB_FILE};
  web::uri url{to_string_t(ADDR)};

  EnvDb env_db{db_path};
  EnvRest env_rest{url, env_db};

  ucout << U("Listening at: ") << url.to_string() << std::endl;

  freeze_thread_until_cin_closed();
}
