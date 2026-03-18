#pragma once
#include "Route.h"
#include <vector>
#include <iostream>
#include <string>

class Bus {
private:
    std::string busNumber;
    Route route;
    int totalSeats;
    std::vector<bool> seats;  

public:
    Bus(const std::string& busNumber, const Route& route, int totalSeats)
        : busNumber(busNumber), route(route), totalSeats(totalSeats),
        seats(totalSeats, false) {
    }

    bool reserveSeat(int seatNumber) {
        if (seatNumber >= 1 && seatNumber <= totalSeats && !seats[seatNumber - 1]) {
            seats[seatNumber - 1] = true;
            return true;
        }
        return false;
    }
    bool cancelSeat(int seatNumber) {
        if (seatNumber >= 1 && seatNumber <= totalSeats && seats[seatNumber - 1]) {
            seats[seatNumber - 1] = false;
            return true;
        }
        return false;
    }

    void displayBusInfo() const {
        std::cout << "Bus Number : " << busNumber << "\n";
        std::cout << "Route      : " << route.getName()
            << " (" << route.getSource() << " -> " << route.getDestination() << ")\n";
        std::cout << "Total Seats: " << totalSeats << "\n";
        std::cout << "Seat Layout (A=Available, X=Reserved):\n";

        for (int i = 0; i < totalSeats; i++) {
            std::cout << "  [" << (i + 1 < 10 ? " " : "") << (i + 1) << ":"
                << (seats[i] ? "X" : "A") << "]";
            if ((i + 1) % 4 == 0) std::cout << "\n";
        }
        std::cout << "\n";
    }

    void displayAvailableSeats() const {
        std::cout << "Available seats on Bus " << busNumber << ": ";
        bool any = false;
        for (int i = 0; i < totalSeats; i++) {
            if (!seats[i]) {
                std::cout << (i + 1) << " ";
                any = true;
            }
        }
        if (!any) std::cout << "None";
        std::cout << "\n";
    }

    void displayBusStatus() const {
        int reserved = 0;
        for (bool s : seats) if (s) reserved++;
        std::cout << "Bus: " << busNumber
            << "  Total: " << totalSeats
            << "  Reserved: " << reserved
            << "  Available: " << (totalSeats - reserved) << "\n";
    }

    int         getTotalSeats() const { return totalSeats; }
    std::string getBusNumber()  const { return busNumber; }
    const Route& getRoute()     const { return route; }

    void markSeatReserved(int seatNumber) {
        if (seatNumber >= 1 && seatNumber <= totalSeats)
            seats[seatNumber - 1] = true;
    }
};