#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include "AVNGenerator.hpp"
#include "AVN.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
using namespace std;
using namespace sf;

class ATCSController
{
private:
    vector<Flight*> activeFlights;
    vector<Flight*> violatingFlights;
    AVNGenerator* avnGenerator;
public:
    ATCSController()
    {
        avnGenerator = new AVNGenerator;
    }
    ~ATCSController()
    {
        delete avnGenerator;
    }
    void addFlight(Flight* flight)
    {
        activeFlights.push_back(flight);
    }
    void detectViolations(Flight* flight)
    {
        string phase = flight->getCurrentPhase();
        int speed = flight->getSpeed();
        int allowedSpeedHigh = 0;
        int allowedSpeedLow = 0;
        bool isViolation = false;

        if(flight->getAssignedRunwayPtr()->getRunwayID() == "RWY-A")
        {
            if(phase == "Holding")
            {
                allowedSpeedHigh = 600;
                allowedSpeedLow = 290;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Approach")
            {
                allowedSpeedHigh = 290;
                allowedSpeedLow = 240;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Landing")
            {
                allowedSpeedHigh = 240;
                allowedSpeedLow = 30;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Taxi")
            {
                allowedSpeedHigh = 30;
                allowedSpeedLow = 10;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "At Gate")
            {
                allowedSpeedHigh = 10;
                allowedSpeedLow = 0;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
        }
        else if(flight->getAssignedRunwayPtr()->getRunwayID() == "RWY-B")
        {
            if(phase == "At Gate")
            {
                allowedSpeedHigh = 10;
                allowedSpeedLow = 0;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Taxi")
            {
                allowedSpeedHigh = 30;
                allowedSpeedLow = 10;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Takeoff Roll")
            {
                allowedSpeedHigh = 290;
                allowedSpeedLow = 30;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Climb")
            {
                allowedSpeedHigh = 463;
                allowedSpeedLow = 290;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
            else if(phase == "Cruise")
            {
                allowedSpeedHigh = 900;
                allowedSpeedLow = 800;
                if(speed > allowedSpeedHigh || speed < allowedSpeedLow)
                {
                    isViolation = true;
                }
            }
        }

        if(isViolation)
        {
            flight->setAVNStatus(true);
            violatingFlights.push_back(flight);

            string a;
            AVN* newAVN = avnGenerator->createAVN(flight, a, speed, allowedSpeedHigh, allowedSpeedLow);

            cout << red << "VIOLATION DETECTED: " << flight->getID()
            << " - " << phase << "phase at" << speed << "km/h (limit: " << allowedSpeedHigh << " - "
            << allowedSpeedLow << "km/h)" << default_text << endl; 
        }
    }

    void displayAnalytics()
    {
        cout << "\n============== AIR TRAFFIC ANALYSIS ================\n";
        cout << "Total Active Flights: " << activeFlights.size() << endl;
        cout << "Flights with Active Violations: " << violatingFlights.size() << endl;
        if(!violatingFlights.empty())
        {
            cout << "Violating Aircraft IDs: " << endl;
            for(auto flight: violatingFlights)
            {
                cout << " - " << flight->getID() << " - " << flight->getCurrentPhase() << endl;
            }
        }
        cout << "=======================================================\n";
    }

    void clearViolation(string flightID)
    {
        for(auto it = violatingFlights.begin(); it != violatingFlights.end();)
        {
            if((*it)->getID() == flightID)
            {
                (*it)->setAVNStatus(false);
                it = violatingFlights.erase(it);
                cout << green << "Violation cleared for flight " << flightID << default_text << endl;
            }
            else 
            {
                it++;
            }
        }
    }

    AVNGenerator* getAVNGenerator()
    {
        return avnGenerator;
    }
};