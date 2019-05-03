#include <sqlite3.h>
#include <stdio.h>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "env-db.h"
#include "utils/db.h"
#include "utils/sample.h"

EnvDb::EnvDb(std::string db_path) : Db(db_path) {
  std::string query =
      "CREATE TABLE IF NOT EXISTS SAMPLES("
      "DATETIME TEXT PRIMARY KEY NOT NULL,"
      "TEMPERATURE REAL NULL,"
      "HUMIDITY REAL NULL,"
      "PRESSURE REAL NULL,"
      "CO2 REAL NULL);";

  exec_query(query);
}

void EnvDb::add_sample(Sample sample) {
  char query[150];
  sprintf(query,
          "INSERT INTO SAMPLES(DATETIME, TEMPERATURE, HUMIDITY, PRESSURE, CO2) "
          "VALUES('%s', '%f', '%f', '%f', '%f')",
          sample.datetime.c_str(), sample.temperature, sample.humidity,
          sample.pressure, sample.co2);

  exec_query(query);
}

std::vector<Sample> EnvDb::get_samples(std::string from, std::string to) {
  std::vector<Sample> res;

  if (from.empty()) {
    from = "0000";
  }

  if (to.empty()) {
    to = "9999";
  }

  char query[150];
  sprintf(query,
          "SELECT DATETIME, TEMPERATURE, HUMIDITY, PRESSURE, CO2 FROM SAMPLES "
          "WHERE DATETIME BETWEEN '%s' AND '%s'",
          from.c_str(), to.c_str());

  exec_query(query, [&](auto stmt) {
    std::string datetime =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    double temperature = sqlite3_column_double(stmt, 1);
    double humidity = sqlite3_column_double(stmt, 2);
    double pressure = sqlite3_column_double(stmt, 3);
    double co2 = sqlite3_column_double(stmt, 4);

    res.push_back({datetime, temperature, humidity, pressure, co2});
  });

  return res;
}

void EnvDb::delete_samples() { exec_query("DELETE FROM SAMPLES"); }