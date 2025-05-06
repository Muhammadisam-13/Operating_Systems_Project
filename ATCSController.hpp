#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include "AVNGenerator.hpp"
#include "PipeAVNGenerator.hpp"  // Include the new header file instead of AVN_Process.cpp
#include "AVN.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>

class ATCSController {
private:
    std::vector<Flight*> activeFlights;
    std::vector<Flight*> violatingFlights;
    PipeAVNGenerator* avnGenerator;
public:
    ATCSController();
    ~ATCSController();
    void addFlight(Flight* flight);
    void detectViolations(Flight* flight);
    void displayAnalytics();
    void clearViolation(std::string flightID);
    AVNGenerator* getAVNGenerator();
};