#include <math.h>
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

Query::Query(sqlite3* _db, std::string query) : db{_db} {
  prepare_statement(query);
}

Query::~Query() { sqlite3_finalize(stmt); }

void Query::prepare_statement(std::string query) {
  if (SQLITE_OK != sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr)) {
    throw std::runtime_error("Failed to prepare statement: " +
                             std::string(sqlite3_errmsg(db)));
  };
}

void Query::step(const std::function<void(sqlite3_stmt*)>& visit) {
  if (done) {
    throw std::runtime_error(
        "Query result is already fully read (check done flag).");
  }

  int ret = sqlite3_step(stmt);
  switch (ret) {
    case SQLITE_ROW:
      if (visit) {
        visit(stmt);
      }
      break;
    case SQLITE_DONE:
      done = true;
      break;
    default:
      throw std::runtime_error("Failed to execute statement: " +
                               std::string(sqlite3_errmsg(db)));
  }
}

void Query::skip() { step(nullptr); }
void Query::exec() { step(nullptr); }

void QueryResultReader::read_scalar(int& res, int row, int col) {
  while (row) {
    skip();
    row--;
  }

  step([&](auto stmt) { res = sqlite3_column_int(stmt, col); });
}

void QueryResultReader::read_all(std::function<void(sqlite3_stmt*)> visit) {
  while (!done) {
    step(visit);
  }
}

// Visits N items from query result.
// - ensures visited items are evenly distributed along the set
// - ensures first and last items visited
// - ensures sequence of visited items preserved
void QueryResultReader::read_evenly(int n, int total,
                                    std::function<void(sqlite3_stmt*)> visit) {
  if (total == 0) {
    return;
  }

  if (n <= 0 || n >= total) {
    for (int i = 0; i < total; i++) {
      step(visit);
    }
    return;
  }

  if (n == 1) {
    for (int i = 0; i < total - 1; i++) {
      skip();
    }

    step(visit);
    return;
  }

  if (n == 2) {
    step(visit);

    for (int i = 1; i < total - 1; i++) {
      skip();
    }

    step(visit);
    return;
  }

  if (n >= 3) {
    step(visit);

    int divider = std::ceil(static_cast<float>(total - 1) / (n - 1));

    int visited = 1;
    for (int i = 1; i < total - 1; i++) {
      if ((i % divider == 0) || ((total - i) == (n - visited))) {
        step(visit);
        visited++;
      } else {
        skip();
      }
    }

    step(visit);
  }
}
