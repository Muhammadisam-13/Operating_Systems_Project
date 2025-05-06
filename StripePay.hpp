#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include "ATCSController.hpp"
#include "AVN.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
using namespace std;
using namespace sf;

class StripePay {
    private:
        ATCSController* atcController = nullptr;
        
    public:
        void setATCController(ATCSController* controller) {
            atcController = controller;
        }
        
        bool processPayment(string avnID, string flightNumber, string aircraftType, double amount) {
            cout << "\n===== STRIPE PAYMENT PROCESSING =====" << endl;
            cout << "Processing payment for AVN: " << avnID << endl;
            cout << "Flight: " << flightNumber << endl;
            cout << "Aircraft Type: " << aircraftType << endl;
            cout << "Amount: PKR " << amount << endl;
            
            // Simulate payment process
            cout << "Enter amount to pay (PKR): ";
            double paidAmount;
            cin >> paidAmount;
            
            if (paidAmount >= amount) {
                cout << green << "Payment Successful!" << default_text << endl;
                
                // Update payment status
                if (atcController != nullptr) {
                    atcController->getAVNGenerator()->updatePaymentStatus(avnID, "paid");
                    atcController->clearViolation(flightNumber);
                }
                
                return true;
            } else {
                cout << red << "Payment Failed! Insufficient amount." << default_text << endl;
                return false;
            }
        }
    };
    