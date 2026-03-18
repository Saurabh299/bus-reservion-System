# bus reservion
Table of Contents
1. Project Overview
2. Features
3. Project Structure
4. Prerequisites
5. Database Setup
6. Build Instructions
7. Usage
8. Payment Flow
9. Class Architecture
10. Troubleshooting

1. Project Overview
The Bus Reservation System is a C++17 console application that allows users to register, log in, browse routes, book seats, make payments, and cancel tickets. All data is persisted to a MySQL database via the MySQL Connector/C++ library.

2. Features
•	User registration and login (case-insensitive username matching)
•	Browse predefined bus routes (Delhi-Pune, Delhi-Lucknow, Lucknow-Delhi)
•	View seat layout with available / reserved status
•	Book a seat with real-time seat locking
•	Payment processing — Credit Card and UPI supported
•	Ticket confirmation with fare summary
•	Cancel a booked ticket with automatic DB rollback
•	View all booked tickets for the logged-in user
•	MySQL-backed persistence across sessions

3. Project Structure

File	Description
Source.cpp	Entry point — creates and runs BusReservationSystem
BusReservationSystem.h	Core system: routes, buses, booking, payment, cancellation
Bus.h	Bus class — seat management, layout display
Route.h	Route class — name, source, destination, distance
Ticket.h	Ticket class — passenger, seat, route, bus
user.h	User class — credentials, ticket list
payment.h	Payment hierarchy — base + CreditCardPayment + UPI
DBConnection.h/.cpp	MySQL connection factory
UserRespository.h/.cpp	All DB operations — users and tickets
CMakeLists.txt	CMake build configuration


4. Prerequisites
Software Required
•	Visual Studio 2019/2022  (or GCC 9+ / Clang 10+ on Linux)
•	MySQL Server 8.0+
•	MySQL Connector/C++ 8.0
•	CMake 3.14+  (optional — VS project file included)
Install MySQL Connector/C++ (Windows)
Download the MSI installer from:
https://dev.mysql.com/downloads/connector/cpp/
Default install path:
C:\Program Files\MySQL\MySQL Connector C++ 8.0\
Visual Studio — Additional Include Directories
Right-click project → Properties → C/C++ → General → Additional Include Directories:
C:\Program Files\MySQL\MySQL Connector C++ 8.0\include
C:\Program Files\MySQL\MySQL Connector C++ 8.0\include\cppconn
Visual Studio — Linker Settings
Linker → General → Additional Library Directories:
C:\Program Files\MySQL\MySQL Connector C++ 8.0\lib64
Linker → Input → Additional Dependencies:
mysqlcppconn.lib
Copy Runtime DLL
copy "C:\Program Files\MySQL\MySQL Connector C++ 8.0\lib64\mysqlcppconn-9-vs14.dll"
     "<ProjectDir>\x64\Debug\"

5. Database Setup
Run the following SQL in MySQL Workbench or the MySQL CLI:
CREATE DATABASE bus_reservation;
USE bus_reservation;

CREATE TABLE users (
    id       INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(100) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL
);

CREATE TABLE tickets (
    id                INT AUTO_INCREMENT PRIMARY KEY,
    username          VARCHAR(100) NOT NULL,
    bus_number        VARCHAR(50),
    seat_number       INT,
    route_name        VARCHAR(100),
    route_source      VARCHAR(100),
    route_destination VARCHAR(100),
    route_distance    INT,
    FOREIGN KEY (username) REFERENCES users(username)
);
DBConnection.cpp — Update Credentials
Open DBConnection.cpp and update the connection string with your MySQL credentials:
con->setSchema("bus_reservation");   // database name
// host: tcp://127.0.0.1:3306
// user: root  (or your MySQL user)
// password: your_password

6. Build Instructions
Option A — Visual Studio (Recommended)
•	Open bus reservion.vcxproj in Visual Studio
•	Set configuration to x64 | Debug (or Release)
•	Press Ctrl+Shift+B to build
•	Press F5 to run
Option B — CMake
cd "<project root>"
rename cmake.txt CMakeLists.txt   # if not already renamed
mkdir build && cd build
cmake ..
cmake --build . --config Release
.\Release\bus_reservation.exe

7. Usage
Main Menu
=== Bus Reservation System ===
1. Register
2. Login
3. Exit
User Menu (after login)
1. Book Ticket
2. Cancel Ticket
3. View My Tickets
4. Logout

8. Payment Flow
After selecting route, bus, and seat, the system prompts for payment:
=== Payment ===
Fare for this journey: Rs.1400
Select payment method:
  1. Credit Card
  2. UPI
Enter choice: 2
Enter UPI ID: john@upi
Processing UPI payment of Rs.1400 to UPI ID: john@upi
Payment of Rs.1400 successful!

==========================================
       TICKET BOOKED SUCCESSFULLY!
==========================================
Passenger : john
Bus       : Bus 101
Seat      : 14
Route     : Route 1 (Delhi -> Pune)
Fare Paid : Rs.1400
==========================================
Fare Calculation
Fare is calculated as Rs.1 per km of route distance.

Route	Distance	Fare
Route 1 — Delhi to Pune	1400 km	Rs. 1400
Route 2 — Delhi to Lucknow	500 km	Rs. 500
Route 3 — Lucknow to Delhi	500 km	Rs. 500


9. Class Architecture

Class	Responsibility
BusReservationSystem	Orchestrates the entire application loop, booking, cancellation
Bus	Manages seat vector; reserve / cancel / display seats
Route	Value object — name, source, destination, distance
Ticket	Value object — passenger, seat, route, bus snapshot
user	Holds credentials and a list of Ticket objects
Payment (abstract)	Base class with makePayment() pure virtual method
CreditCardPayment	Concrete payment — card number, holder, expiry
UPI	Concrete payment — UPI ID
UserRepository	All SQL operations — save/load users and tickets
DBConnection	Static factory returning a sql::Connection*


10. Troubleshooting

Error	Cause	Fix
Cannot open source file 'cppconn/driver.h'	MySQL Connector/C++ not installed or path not set	Install connector; add include path in VS project properties
LNK2019 unresolved external mysqlcppconn	mysqlcppconn.lib not linked	Add mysqlcppconn.lib to Linker → Input → Additional Dependencies
Access denied for user 'root'	Wrong MySQL credentials in DBConnection.cpp	Update username/password in DBConnection.cpp
cmake not recognized	CMake not installed or not in PATH	Install CMake from cmake.org and tick 'Add to PATH' during install
Payment step skipped	Running old compiled binary	Recompile after replacing BusReservationSystem.h
mysqlcppconn-9-vs14.dll not found	Runtime DLL missing from output directory	Copy DLL from Connector lib64 folder to x64\Debug\

