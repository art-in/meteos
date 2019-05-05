#include <sqlite3.h>
#include <stdexcept>
#include <string>

#include "db.h"

Db::Db(std::string db_path) {
  if (SQLITE_OK != sqlite3_open(db_path.c_str(), &db)) {
    std::string errmsg = sqlite3_errmsg(db);
    throw std::runtime_error("Failed to open database (" + db_path +
                             ") : " + errmsg);
  }
}

Db::~Db() { sqlite3_close_v2(db); }

sqlite3_stmt* Db::prepare_statement(std::string query) {
  sqlite3_stmt* statement;
  if (SQLITE_OK !=
      sqlite3_prepare_v2(db, query.c_str(), -1, &statement, nullptr)) {
    throw std::runtime_error("Failed to prepare statement: " +
                             std::string(sqlite3_errmsg(db)));
  };

  return statement;
}

void Db::exec_query(std::string query) { exec_query(query, nullptr); }

void Db::exec_query(std::string query,
                    const std::function<void(sqlite3_stmt*)>& step) {
  auto stmt = prepare_statement(query);

  bool done = false;
  while (!done) {
    int ret = sqlite3_step(stmt);
    switch (ret) {
      case SQLITE_ROW:
        step(stmt);
        break;
      case SQLITE_DONE:
        done = true;
        return;
        break;
      default:
        throw std::runtime_error("Failed to execute statement: " +
                                 std::string(sqlite3_errmsg(db)));
    }
  }

  sqlite3_finalize(stmt);
}