#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "AirlinePortal.hpp"
#include "Colors.hpp"
#include "AVN.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
using namespace std;
using namespace sf;

class AVNGenerator
{
    vector<AVN*> issuedAVNs;
    AirlinePortal* airlinePortal;
public:
    AVNGenerator()
    {
        airlinePortal = new AirlinePortal();
    }

    ~AVNGenerator()
    {
        for(auto avn : issuedAVNs)
        {
            delete avn;
        }
        delete airlinePortal;
    }

    AVN* createAVN(Flight* flight, string airlinename, int recordedSpeed, int allowedHigh, int allowedLow)
    {
        AVN* newAVN = new AVN(flight->getID(), airlinename, flight->getFlightType(), recordedSpeed, allowedHigh, allowedLow);

        issuedAVNs.push_back(newAVN);
        airlinePortal->addAVN(newAVN);
        cout << "AVN Generated: " << newAVN->getAVNID() << "for" << flight->getID() << endl;
        return newAVN;
    }

    void updatePaymentStatus(string avnID, string status)
    {
        for(auto avn : issuedAVNs)
        {
            if(avn->getAVNID() == avnID)
            {
                avn->setPaymentStatus(status);
                airlinePortal->updateAVNStatus(avnID, status);
                cout << "Payment Status updated for AVN" << avnID << " to " << status << endl;
                break;
            }
        }
    }

    AirlinePortal* getAirlinePortal()
    {
        return airlinePortal;
    }

    vector<AVN*> getissuedAVNs()
    {
        return issuedAVNs;
    }
};