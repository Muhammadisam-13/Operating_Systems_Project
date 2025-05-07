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
public:

    ~AVNGenerator()
    {
        for(auto avn : issuedAVNs)
        {
            delete avn;
        }
    }

    AVN* createAVN(Flight* flight, int recordedSpeed, int allowedHigh, int allowedLow)
    {
        string airlinename = "Unknown";
        if(flight->getParentAirline() != NULL)
        {
            airlinename = flight->getParentAirline()->getName();
        }
        AVN* newAVN = new AVN(flight->getID(), airlinename, flight->getFlightType(), recordedSpeed, allowedHigh, allowedLow);

        issuedAVNs.push_back(newAVN);
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
                cout << "Payment Status updated for AVN" << avnID << " to " << status << endl;
                break;
            }
        }
    }

    vector<AVN*> getissuedAVNs()
    {
        return issuedAVNs;
    }
};