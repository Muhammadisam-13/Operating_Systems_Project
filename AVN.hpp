#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
using namespace std;
using namespace sf;

class AVN
{
private:
    string avnID;
    string flightNumber;
    string AircraftType;
    string flightType;
    int recordedSpeed;
    int allowedSpeedLow;
    int allowedSpeedHigh;
    string issuanceDateTime;
    double baseAmount;
    double serviceFee;
    double totalAmount;
    string paymentStatus;     string airlineName;

    string dueDate;
public:
    AVN(string flightID, string airline, string type, int recorded, int permissibleLow, int permissibleHigh) 
    {
        // Generate unique AVN ID (timestamp + flight ID)
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y%m%d%H%M%S", localtime(&in_time_t));
        
        this->avnID = string(timeStr) + "-" + flightID;
        this->airlineName = airline;
        this->flightNumber = flightID;
        this->flightType = type;
        this->recordedSpeed = recorded;
        this->allowedSpeedLow = permissibleLow;
        this->allowedSpeedHigh = permissibleHigh;
        
        // Set issuance date/time
        char dateTimeStr[30];
        strftime(dateTimeStr, sizeof(dateTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&in_time_t));
        this->issuanceDateTime = string(dateTimeStr);
        
        // Set due date (3 days from now)
        auto dueTime = now + chrono::hours(72);
        auto due_time_t = chrono::system_clock::to_time_t(dueTime);
        char dueDateStr[20];
        strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d", localtime(&due_time_t));
        this->dueDate = string(dueDateStr);
        
        // Calculate fines
        if (type == "Commercial" || type == "Passenger") 
        {
            this->baseAmount = 500000.0;
        } else 
        {  // Cargo or Military
            this->baseAmount = 700000.0;
        }
        
        this->serviceFee = this->baseAmount * 0.15;
        this->totalAmount = this->baseAmount + this->serviceFee;
        this->paymentStatus = "unpaid";
    }

    string getAVNID() const { return avnID; }
    string getAirlineName() const { return airlineName; }
    string getFlightNumber() const { return flightNumber; }
    string getflightType() const { return flightType; }
    string getAircraftType() const { return AircraftType; }
    double getTotalAmount() const { return totalAmount; }
    string getPaymentStatus() const { return paymentStatus; }
    string getDueDate() const { return dueDate; }
    string getIssuanceDateTime() const { return issuanceDateTime; }
    
    // Setters
    void setPaymentStatus(string status) { 
        this->paymentStatus = status; 
    }

    void display() 
    {
        cout << "===== AVN DETAILS =====" << endl;
        cout << "AVN ID: " << avnID << endl;
        cout << "Airline: " << airlineName << endl;
        cout << "Flight: " << flightNumber << endl;
        cout << "Aircraft Type: " << flightType << endl;
        cout << "Speed Recorded: " << recordedSpeed << " km/h" << endl;
        cout << "Speed Limit: " << allowedSpeedLow << " km/h to " << allowedSpeedHigh << "km/h" << endl;
        cout << "Issued On: " << issuanceDateTime << endl;
        cout << "Due Date: " << dueDate << endl;
        cout << "Base Amount: PKR " << baseAmount << endl;
        cout << "Service Fee: PKR " << serviceFee << endl;
        cout << "Total Amount: PKR " << totalAmount << endl;
        cout << "Payment Status: " << paymentStatus << endl;
        cout << "======================" << endl;
    }
};