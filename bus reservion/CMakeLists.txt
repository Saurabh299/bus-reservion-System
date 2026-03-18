cmake_minimum_required(VERSION 3.14)
project(BusReservationSystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Sources ───────────────────────────────────────────────────────────────────
add_executable(bus_reservation
    Source.cpp
    DBConnection.cpp
    UserRespository.cpp
)

# ── Headers (so IDEs pick them up) ───────────────────────────────────────────
target_sources(bus_reservation PRIVATE
    BusReservationSystem.h
    Bus.h
    Route.h
    Ticket.h
    user.h
    payment.h
    DBConnection.h
    UserRespository.h
)

# ── MySQL Connector/C++ ───────────────────────────────────────────────────────
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(MYSQLCPP mysqlcppconn)
endif()

if(MYSQLCPP_FOUND)
    target_include_directories(bus_reservation PRIVATE ${MYSQLCPP_INCLUDE_DIRS})
    target_link_libraries(bus_reservation PRIVATE ${MYSQLCPP_LIBRARIES})
else()
    # Fallback: common install paths on Linux / Windows
    target_include_directories(bus_reservation PRIVATE
        /usr/include/cppconn
        /usr/local/include/cppconn
        "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include"
        "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include/cppconn"
    )
    target_link_directories(bus_reservation PRIVATE
        /usr/lib
        /usr/local/lib
        "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64"
    )
    target_link_libraries(bus_reservation PRIVATE mysqlcppconn)
endif()

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(bus_reservation PRIVATE /W4)
else()
    target_compile_options(bus_reservation PRIVATE -Wall -Wextra -Wpedantic)
endif()