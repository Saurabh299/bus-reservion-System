#pragma once
#include "Bus.h"
#include <string>

class Ticket {
private:
    std::string passengerName;
    int         seatNumber;
    Route       route;
    Bus         bus;

public:
    Ticket(const std::string& passengerName, int seatNumber,
        const Route& route, const Bus& bus)
        : passengerName(passengerName), seatNumber(seatNumber),
        route(route), bus(bus) {
    }

    std::string  getPassengerName() const { return passengerName; }
    int          getSeatNumber()    const { return seatNumber; }
    const Route& getRoute()         const { return route; }
    const Bus& getBus()           const { return bus; }
};