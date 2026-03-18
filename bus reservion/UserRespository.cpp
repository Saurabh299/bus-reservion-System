#include "DBConnection.h"

#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

#include <iostream>
#include <stdexcept>
#include <memory>   // unique_ptr for RAII
#include "UserRespository.h"

// ─── RAII helpers ────────────────────────────────────────────────────────────
// Wrap raw pointers so they are always deleted even when an exception is thrown.

struct ConGuard {
    sql::Connection* ptr;
    explicit ConGuard(sql::Connection* p) : ptr(p) {}
    ~ConGuard() { delete ptr; }
};

// ─── saveUser ────────────────────────────────────────────────────────────────

void UserRepository::saveUser(const user& u) {
    ConGuard cg(DBConnection::connect());

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "INSERT INTO users (username, password) VALUES (?, ?)"
            )
        );
        pstmt->setString(1, u.getUserName());
        pstmt->setString(2, u.getPassword());  // TODO: hash before storing (bcrypt / SHA-256)
        pstmt->execute();
        std::cout << "User saved to database.\n";
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error saving user: " << e.what() << "\n";
        throw std::runtime_error("Failed to save user.");
    }
}

// ─── userExists ──────────────────────────────────────────────────────────────

bool UserRepository::userExists(const std::string& userName) {
    ConGuard cg(DBConnection::connect());
    bool exists = false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "SELECT COUNT(*) FROM users WHERE LOWER(username) = LOWER(?)"
            )
        );
        pstmt->setString(1, userName);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next())
            exists = res->getInt(1) > 0;
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error checking user existence: " << e.what() << "\n";
    }

    return exists;
}

// ─── authenticateUser ────────────────────────────────────────────────────────

bool UserRepository::authenticateUser(const std::string& userName,
    const std::string& password) {
    ConGuard cg(DBConnection::connect());
    bool ok = false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "SELECT COUNT(*) FROM users "
                "WHERE LOWER(username) = LOWER(?) AND password = ?"
            )
        );
        pstmt->setString(1, userName);
        pstmt->setString(2, password);  // TODO: compare hashed passwords

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next())
            ok = res->getInt(1) > 0;
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error authenticating user: " << e.what() << "\n";
    }

    return ok;
}

// ─── saveTicket ──────────────────────────────────────────────────────────────

void UserRepository::saveTicket(const std::string& userName, const Ticket& t) {
    ConGuard cg(DBConnection::connect());

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "INSERT INTO tickets "
                "(username, bus_number, seat_number, route_name, "
                " route_source, route_destination, route_distance) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)"
            )
        );
        pstmt->setString(1, userName);
        pstmt->setString(2, t.getBus().getBusNumber());
        pstmt->setInt(3, t.getSeatNumber());
        pstmt->setString(4, t.getRoute().getName());
        pstmt->setString(5, t.getRoute().getSource());
        pstmt->setString(6, t.getRoute().getDestination());
        pstmt->setInt(7, t.getRoute().getDistance());
        pstmt->execute();
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error saving ticket: " << e.what() << "\n";
        throw std::runtime_error("Failed to save ticket.");
    }
}

// ─── deleteTicket ────────────────────────────────────────────────────────────

void UserRepository::deleteTicket(const std::string& userName,
    const std::string& busNumber,
    int seatNumber) {
    ConGuard cg(DBConnection::connect());

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "DELETE FROM tickets "
                "WHERE username = ? AND bus_number = ? AND seat_number = ? "
                "LIMIT 1"
            )
        );
        pstmt->setString(1, userName);
        pstmt->setString(2, busNumber);
        pstmt->setInt(3, seatNumber);
        pstmt->execute();
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error deleting ticket: " << e.what() << "\n";
        throw std::runtime_error("Failed to delete ticket.");
    }
}

// ─── loadTickets ─────────────────────────────────────────────────────────────

void UserRepository::loadTickets(user& u,
    const std::vector<Route>& routes,
    const std::vector<Bus>& buses) {
    ConGuard cg(DBConnection::connect());

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            cg.ptr->prepareStatement(
                "SELECT bus_number, seat_number, route_name, "
                "       route_source, route_destination, route_distance "
                "FROM tickets WHERE username = ?"
            )
        );
        pstmt->setString(1, u.getUserName());

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::string busNum = res->getString("bus_number");
            int         seat = res->getInt("seat_number");
            std::string routeName = res->getString("route_name");
            std::string routeSrc = res->getString("route_source");
            std::string routeDst = res->getString("route_destination");
            int         routeDist = res->getInt("route_distance");

            // Find matching Route from in-memory list (or rebuild from DB columns)
            Route ticketRoute(routeName, routeSrc, routeDst, routeDist);

            // Find matching Bus from in-memory list
            const Bus* matchedBus = nullptr;
            for (const auto& b : buses) {
                if (b.getBusNumber() == busNum) {
                    matchedBus = &b;
                    break;
                }
            }

            if (!matchedBus) {
                std::cerr << "Warning: bus " << busNum
                    << " from DB not found in memory. Skipping ticket.\n";
                continue;
            }

            Ticket t(u.getUserName(), seat, ticketRoute, *matchedBus);
            u.addTicket(t);
        }
    }
    catch (const sql::SQLException& e) {
        std::cerr << "DB error loading tickets: " << e.what() << "\n";
    }
}