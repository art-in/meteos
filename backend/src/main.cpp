#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "env-db.h"
#include "env-rest.h"
#include "utils/file-logger.h"
#include "utils/utils.h"

using std::filesystem::path;
using utility::conversions::to_string_t;
using utility::conversions::to_utf8string;

constexpr auto URL = "http://0.0.0.0:3000";
constexpr auto DB_FILE = "/data/backend.db";
constexpr auto LOG_FILE = "/data/backend.log";

int main(int argc, char* argv[]) {
  std::string bin_path{path(argv[0]).parent_path().u8string()};
  std::string log_path{bin_path + LOG_FILE};
  std::string db_path{bin_path + DB_FILE};
  web::uri url{to_string_t(URL)};

  FileLogger logger{log_path};

  try {
    EnvDb env_db{db_path};
    EnvRest env_rest{url, env_db, logger};

    log_service_start(logger, url, db_path, log_path);
    freeze_thread_until_cin_closed();
    log_service_stop(logger);

  } catch (std::exception& e) {
    log_service_start_failed(logger, e);
    return 1;
  }
}
