#pragma once

#include <sqlite3.h>
#include <functional>
#include <string>

class Db {
 protected:
  Db(std::string db_path);
  ~Db();

  sqlite3 *db;
};

class Query {
 public:
  Query(sqlite3 *db, std::string query);
  ~Query();

  Query(const Query &) = delete;
  Query(const Query &&) = delete;
  Query &operator=(const Query &) = delete;
  Query &operator=(const Query &&) = delete;

  void exec();

 protected:
  sqlite3 *db;
  sqlite3_stmt *stmt;
  bool done = false;

  void step(const std::function<void(sqlite3_stmt *)> &);
  void skip();

 private:
  void prepare_statement(std::string query);
};

class QueryResultReader : Query {
 public:
  QueryResultReader(sqlite3 *db, std::string query) : Query(db, query) {}

  void read_scalar(int &, int row = 0, int col = 0);
  void read_all(std::function<void(sqlite3_stmt *)> visit);
  void read_evenly(int n, int total, std::function<void(sqlite3_stmt *)> visit);
};