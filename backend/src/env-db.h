#pragma once

#include <sqlite3.h>
#include <functional>
#include <string>
#include <vector>

#include "utils/db.h"
#include "utils/sample.h"

class EnvDb : Db {
 public:
  EnvDb(std::string db_path);
  ~EnvDb() { sqlite3_close(db); };

  void add_sample(Sample sample);
  std::vector<Sample> get_samples();
};