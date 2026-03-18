#pragma once
#include "user.h"
#include "payment.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "UserRespository.h"

class BusReservationSystem {
private:
    std::vector<user>  users;   // in-memory cache of logged-in / registered users
    std::vector<Route> routes;
    std::vector<Bus>   buses;
    UserRepository     repo;


    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    // Returns nullptr if not found in the in-memory list
    user* findUserInMemory(const std::string& userName) {
        std::string low = toLower(userName);
        for (auto& u : users)
            if (toLower(u.getUserName()) == low)
                return &u;
        return nullptr;
    }

    // Builds the list of main-vector indices whose route matches selectedRoute (1-based)
    std::vector<int> getBusIndicesForRoute(int selectedRoute) const {
        std::vector<int> indices;
        const std::string& routeName = routes[selectedRoute - 1].getName();
        for (int i = 0; i < (int)buses.size(); i++)
            if (buses[i].getRoute().getName() == routeName)
                indices.push_back(i);
        return indices;
    }

    void displayRoutes() const {
        std::cout << "\n=== Available Routes ===\n";
        for (int i = 0; i < (int)routes.size(); i++)
            std::cout << i + 1 << ". " << routes[i].getName()
            << " (" << routes[i].getSource()
            << " -> " << routes[i].getDestination()
            << ")  " << routes[i].getDistance() << " km\n";
    }

    void displayBusesForRoute(int selectedRoute) const {
        std::cout << "\nBuses for " << routes[selectedRoute - 1].getName() << ":\n";
        std::vector<int> idx = getBusIndicesForRoute(selectedRoute);
        for (int i = 0; i < (int)idx.size(); i++) {
            std::cout << i + 1 << ". ";
            buses[idx[i]].displayBusInfo();
        }
    }

    // Read an integer from stdin; return false on bad input
    static bool readInt(int& out, int lo, int hi) {
        std::string s;
        std::cin >> s;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        try {
            out = std::stoi(s);
            return out >= lo && out <= hi;
        }
        catch (...) { return false; }
    }

public:
    BusReservationSystem() {
        // Seed routes and buses
        routes.push_back(Route("Route 1", "Delhi", "Pune", 1400));
        routes.push_back(Route("Route 2", "Delhi", "Lucknow", 500));
        routes.push_back(Route("Route 3", "Lucknow", "Delhi", 500));

        buses.push_back(Bus("Bus 101", routes[0], 40));
        buses.push_back(Bus("Bus 102", routes[1], 40));
        buses.push_back(Bus("Bus 201", routes[2], 50));
    }


    void registerUser(const std::string& userName, const std::string& password) {
        if (userName.empty() || password.empty()) {
            std::cout << "Username and password cannot be empty!\n";
            return;
        }
        // Check DB first (handles restarts / multiple sessions)
        if (repo.userExists(userName)) {
            std::cout << "Username already exists! Please choose a different username.\n";
            return;
        }
        user newUser(userName, password);
        repo.saveUser(newUser);          // persist to DB
        users.push_back(newUser);        // cache in memory
        std::cout << "User registered successfully!\n";
    }

    // Returns pointer to the logged-in user, or nullptr on failure
    user* login(const std::string& userName, const std::string& password) {
        // Verify against DB
        if (!repo.authenticateUser(userName, password)) {
            std::cout << "Authentication failed! Invalid username or password.\n";
            return nullptr;
        }
        std::cout << "Authentication successful!\n";

        // Ensure user is in memory cache
        user* u = findUserInMemory(userName);
        if (!u) {
            users.push_back(user(userName, password));
            u = &users.back();
        }

        // Load tickets from DB (marks seats reserved in buses too)
        repo.loadTickets(*u, routes, buses);

        for (int i = 0; i < u->numberOfTickets(); i++) {
            const Ticket& t = u->getTicket(i + 1);
            for (auto& b : buses) {
                if (b.getBusNumber() == t.getBus().getBusNumber()) {
                    b.markSeatReserved(t.getSeatNumber());
                    break;
                }
            }
        }
        return u;
    }


    void bookTicket(user& currentUser) {
        displayRoutes();

        int selectedRoute;
        std::cout << "Select a Route (1-" << routes.size() << "): ";
        if (!readInt(selectedRoute, 1, (int)routes.size())) {
            std::cout << "Invalid route selection!\n"; return;
        }

        std::vector<int> routeBusIdx = getBusIndicesForRoute(selectedRoute);
        if (routeBusIdx.empty()) {
            std::cout << "No buses available for this route.\n"; return;
        }
        displayBusesForRoute(selectedRoute);

        int selectedBus;
        std::cout << "Select a Bus (1-" << routeBusIdx.size() << "): ";
        if (!readInt(selectedBus, 1, (int)routeBusIdx.size())) {
            std::cout << "Invalid bus selection!\n"; return;
        }

        int actualBusIdx = routeBusIdx[selectedBus - 1];
        buses[actualBusIdx].displayAvailableSeats();

        int seatNumber;
        std::cout << "Enter Seat Number (1-" << buses[actualBusIdx].getTotalSeats() << "): ";
        if (!readInt(seatNumber, 1, buses[actualBusIdx].getTotalSeats())) {
            std::cout << "Invalid seat number!\n"; return;
        }

        if (!buses[actualBusIdx].reserveSeat(seatNumber)) {
            std::cout << "Seat " << seatNumber << " is already reserved! Choose another.\n";
            return;
        }

        // Fare = Rs.1 per km (simple formula; adjust as needed)
        int fare = routes[selectedRoute - 1].getDistance();

        std::cout << "\n=== Payment ===\n";
        std::cout << "Fare for this journey: Rs." << fare << "\n";
        std::cout << "Select payment method:\n";
        std::cout << "  1. Credit Card\n";
        std::cout << "  2. UPI\n";
        std::cout << "Enter choice: ";

        int payChoice;
        if (!readInt(payChoice, 1, 2)) {
            std::cout << "Invalid payment choice! Booking cancelled.\n";
            buses[actualBusIdx].cancelSeat(seatNumber);   // release the seat
            return;
        }

        std::unique_ptr<Payment> payment;

        if (payChoice == 1) {
            std::string cardNum, holderName, expiry;
            std::cout << "Enter Card Number    : "; std::getline(std::cin, cardNum);
            std::cout << "Enter Card Holder    : "; std::getline(std::cin, holderName);
            std::cout << "Enter Expiry (MM/YY) : "; std::getline(std::cin, expiry);
            payment = std::make_unique<CreditCardPayment>(fare, cardNum, holderName, expiry);
        }
        else {
            std::string upiId;
            std::cout << "Enter UPI ID: "; std::getline(std::cin, upiId);
            payment = std::make_unique<UPI>(fare, upiId);
        }

        payment->makePayment();
        std::cout << "Payment of Rs." << fare << " successful!\n";

        Ticket newTicket(currentUser.getUserName(), seatNumber,
            routes[selectedRoute - 1], buses[actualBusIdx]);
        currentUser.addTicket(newTicket);

        // Persist to DB
        try {
            repo.saveTicket(currentUser.getUserName(), newTicket);
        }
        catch (const std::exception& e) {
            // Roll back in-memory state on DB failure
            buses[actualBusIdx].cancelSeat(seatNumber);
            currentUser.removeTicket(currentUser.numberOfTickets() - 1);
            std::cout << "Booking failed: " << e.what() << "\n";
            return;
        }

        std::cout << "\n==========================================\n";
        std::cout << "       TICKET BOOKED SUCCESSFULLY!        \n";
        std::cout << "==========================================\n";
        std::cout << "Passenger : " << currentUser.getUserName() << "\n";
        std::cout << "Bus       : " << buses[actualBusIdx].getBusNumber() << "\n";
        std::cout << "Seat      : " << seatNumber << "\n";
        std::cout << "Route     : " << routes[selectedRoute - 1].getName()
            << " (" << routes[selectedRoute - 1].getSource()
            << " -> " << routes[selectedRoute - 1].getDestination() << ")\n";
        std::cout << "Fare Paid : Rs." << fare << "\n";
        std::cout << "==========================================\n";
    }

    void cancelTicket(user& currentUser) {
        if (currentUser.numberOfTickets() == 0) {
            std::cout << "No tickets to cancel!\n"; return;
        }

        currentUser.printAllTicket();
        int ticketNum;
        std::cout << "Select ticket to cancel (1-" << currentUser.numberOfTickets() << "): ";
        if (!readInt(ticketNum, 1, currentUser.numberOfTickets())) {
            std::cout << "Invalid ticket number!\n"; return;
        }

        Ticket ticketToCancel = currentUser.getTicket(ticketNum);  // copy before removal

        for (auto& b : buses) {
            if (b.getBusNumber() == ticketToCancel.getBus().getBusNumber()) {
                b.cancelSeat(ticketToCancel.getSeatNumber());
                break;
            }
        }
        currentUser.removeTicket(ticketNum - 1);

        try {
            repo.deleteTicket(currentUser.getUserName(),
                ticketToCancel.getBus().getBusNumber(),
                ticketToCancel.getSeatNumber());
        }
        catch (const std::exception& e) {
            std::cout << "Warning: ticket removed locally but DB delete failed: "
                << e.what() << "\n";
        }

        std::cout << "Ticket cancelled successfully!\n";
    }


    void run() {
        while (true) {
            std::cout << "\n=== Bus Reservation System ===\n";
            std::cout << "1. Register\n2. Login\n3. Exit\nEnter choice: ";

            int choice;
            if (!readInt(choice, 1, 3)) {
                std::cout << "Invalid input! Please enter 1, 2, or 3.\n";
                continue;
            }

            if (choice == 3) {
                std::cout << "Exiting. Goodbye!\n";
                break;
            }

            std::string userName, password;
            std::cout << "Enter Username: ";
            std::getline(std::cin, userName);
            std::cout << "Enter Password: ";
            std::getline(std::cin, password);

            if (choice == 1) {
                registerUser(userName, password);
            }
            else {  
                user* loggedIn = login(userName, password);
                if (!loggedIn) continue;

                while (true) {
                    std::cout << "\n1. Book Ticket\n2. Cancel Ticket\n"
                        "3. View My Tickets\n4. Logout\nEnter choice: ";
                    int uc;
                    if (!readInt(uc, 1, 4)) {
                        std::cout << "Invalid choice!\n"; continue;
                    }
                    if (uc == 1) bookTicket(*loggedIn);
                    else if (uc == 2) cancelTicket(*loggedIn);
                    else if (uc == 3) loggedIn->printAllTicket();
                    else  break;  
                }
            }
        }
    }
};