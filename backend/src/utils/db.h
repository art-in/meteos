#pragma once

#include <sqlite3.h>
#include <functional>
#include <string>

class Db {
 protected:
  Db(std::string db_path);
  ~Db();

  sqlite3 *db;
  sqlite3_stmt *prepare_statement(std::string query);

  void exec_query(std::string query);
  void exec_query(std::string query,
                  const std::function<void(sqlite3_stmt *)> &step);
};