#include "DBConnection.h"

#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/connection.h>

#include <cstdlib>      // getenv
#include <stdexcept>
#include <string>

static std::string getEnv(const char* key, const char* fallback = "") {
    const char* val = std::getenv(key);
    return val ? std::string(val) : std::string(fallback);
}

sql::Connection* DBConnection::connect() {
    std::string host = getEnv("DB_HOST", "tcp://127.0.0.1:3306");
    std::string user = getEnv("DB_USER", "root");
    std::string pass = getEnv("DB_PASS", "");          
    std::string schema = getEnv("DB_NAME", "bus_reservation");

    if (pass.empty()) {
        throw std::runtime_error(
            "DB_PASS environment variable is not set. "
            "Please set it before running the application.");
    }

    try {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        sql::Connection* con = driver->connect(host, user, pass);
        con->setSchema(schema);
        return con;
    }
    catch (const sql::SQLException& e) {
        throw std::runtime_error(
            std::string("Database connection failed: ") + e.what());
    }
}