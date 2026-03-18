#pragma once
#include "user.h"
#include <string>
#include <vector>

class UserRepository {
public:
    void saveUser(const user& u);

    bool userExists(const std::string& userName);

    bool authenticateUser(const std::string& userName, const std::string& password);

    void saveTicket(const std::string& userName, const Ticket& t);

    void deleteTicket(const std::string& userName, const std::string& busNumber, int seatNumber);

    void loadTickets(user& u,
        const std::vector<Route>& routes,
        const std::vector<Bus>& buses);
};