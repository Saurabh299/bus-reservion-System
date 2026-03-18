#pragma once
#include <string>

class Route {
private:
    std::string name;
    std::string source;
    std::string destination;
    int distance;

public:
    Route(const std::string& name, const std::string& source,
        const std::string& destination, int distance)
        : name(name), source(source), destination(destination), distance(distance) {
    }

    std::string getName()        const { return name; }
    std::string getSource()      const { return source; }
    std::string getDestination() const { return destination; }
    int         getDistance()    const { return distance; }
};