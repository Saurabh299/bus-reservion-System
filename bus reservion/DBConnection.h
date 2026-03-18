#pragma once
#include <cppconn/connection.h>
#include <string>

class DBConnection {
public:
    // Returns a new heap-allocated connection. Caller must delete it.
    static sql::Connection* connect();
};