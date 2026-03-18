#pragma once
#include "Ticket.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>

class user {
private:
    std::string userName;
    std::string password;
    std::vector<Ticket> tickets;

public:
    user(const std::string& userName, const std::string& password)
        : userName(userName), password(password) {
    }

    std::string getUserName() const { return userName; }
    std::string getPassword() const { return password; }

    void addTicket(const Ticket& t) {
        tickets.push_back(t);
    }

    void removeTicket(int index) {
        if (index >= 0 && index < (int)tickets.size())
            tickets.erase(tickets.begin() + index);
    }

    int numberOfTickets() const { return (int)tickets.size(); }

    void printAllTicket() const {
        if (tickets.empty()) {
            std::cout << "No tickets booked.\n";
            return;
        }
        for (int i = 0; i < (int)tickets.size(); i++) {
            const Ticket& t = tickets[i];
            std::cout << "---- Ticket " << (i + 1) << " ----\n";
            std::cout << "Bus    : " << t.getBus().getBusNumber() << "\n";
            std::cout << "Seat   : " << t.getSeatNumber() << "\n";
            std::cout << "Name   : " << t.getPassengerName() << "\n";
            std::cout << "Route  : " << t.getRoute().getName()
                << " (" << t.getRoute().getSource()
                << " -> " << t.getRoute().getDestination() << ")\n";
        }
    }

    Ticket& getTicket(int index) {
        if (index < 1 || index >(int)tickets.size())
            throw std::out_of_range("Invalid ticket index");
        return tickets[index - 1];
    }
};