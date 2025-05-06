#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include "AVNGenerator.hpp"
#include "AVN.hpp"
#include "StripePay.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
using namespace std;
using namespace sf;

class AirlinePortal
{
    unordered_map<string, vector<AVN*>> airlineAVNs; //airline->AVNs
    StripePay* stripePay;
public:
    AirlinePortal()
    {
        stripePay = new StripePay;
    }

    ~AirlinePortal()
    {
        delete stripePay;
    }

    void addAVN(AVN* avn)
    {
        string airline = avn->getAirlineName();
        airlineAVNs[airline].push_back(avn);
    }

    void displayAVNs(string airlineName)
    {
        cout << "\n==============Airline Portal: " << airlineName << "===============\n";
        if(airlineAVNs.find(airlineName) != airlineAVNs.end())
        {
            if(airlineAVNs[airlineName].empty())
            {
                cout << "No violations found for " << airlineName << endl;
            }
            else
            {
                cout << "Active and Past AVNs" << endl;
                for(auto avn : airlineAVNs[airlineName])
                {
                    cout << "AVN ID: " << avn->getAVNID() << endl;
                    cout << "Flight: " << avn->getFlightNumber() << endl;
                    cout << "Payment Status:" << avn->getPaymentStatus() << endl;
                    cout << "Total Amount: PKR" << avn->getTotalAmount() << endl;
                    cout << "Due Date" << avn->getDueDate() << endl;
                    cout << "----------------------------" << endl; 
                }
            }
        }
        else
        {
            cout << "No airlines found with name: " << airlineName << endl;
        }
        cout << "\m==========================================================\n";
    }

    void updateAVNStatus(string avnID, string status) {
        for (auto& pair : airlineAVNs) {
            for (auto avn : pair.second) {
                if (avn->getAVNID() == avnID) {
                    avn->setPaymentStatus(status);
                    cout << green << "Airline Portal: Updated AVN " << avnID 
                         << " status to " << status << default_text << endl;
                    return;
                }
            }
        }
    }
    
    bool payAVN(string avnID) {
        for (auto& pair : airlineAVNs) {
            for (auto avn : pair.second) {
                if (avn->getAVNID() == avnID) {
                    if (avn->getPaymentStatus() == "unpaid") {
                        // Process payment through StripePay
                        bool success = stripePay->processPayment(
                            avn->getAVNID(),
                            avn->getFlightNumber(),
                            avn->getAircraftType(),
                            avn->getTotalAmount()
                        );
                        
                        if (success) {
                            return true;
                        }
                    } else {
                        cout << yellow << "This AVN is already " << avn->getPaymentStatus() << default_text << endl;
                    }
                    return false;
                }
            }
        }
        
        cout << red << "AVN not found with ID: " << avnID << default_text << endl;
        return false;
    }
    
    StripePay* getStripePay() {
        return stripePay;
    }
};