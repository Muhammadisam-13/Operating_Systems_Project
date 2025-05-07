#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <map>
#include <signal.h>
#include <pthread.h>
#include "Colors.hpp"

using namespace std;

#define ATC_TO_AIRLINE_PIPE "atc_to_airline_pipe"
#define AIRLINE_TO_ATC_PIPE "airline_to_atc_pipe"

struct AVNMessage 
{
    char avnID[50];
    char flightNumber[20];
    char airlineName[50];
    char flightType[20];
    int recordedSpeed;
    int allowedSpeedLow;
    int allowedSpeedHigh;
    char issuanceDateTime[30];
    double baseAmount;
    double serviceFee;
    double totalAmount;
    char paymentStatus[20];
    char dueDate[20];
};

struct StatusMessage 
{
    char avnID[50];
    char status[20];
    char flightNumber[20]; // Added to help with flight identification
};

void sendStatusUpdate(const char* avnID, const char* status, const char* flightNumber);
void* receiveAVNs(void* arg);

void cleanupPipes(int signal) 
{
    cout << "Cleaning up pipes..." << endl;
    unlink(ATC_TO_AIRLINE_PIPE);
    unlink(AIRLINE_TO_ATC_PIPE);
    exit(0);
}

class AVN 
{
private:
    string avnID;
    string flightNumber;
    string airlineName;
    string flightType;
    int recordedSpeed;
    int allowedSpeedLow;
    int allowedSpeedHigh;
    string issuanceDateTime;
    double baseAmount;
    double serviceFee;
    double totalAmount;
    string paymentStatus;
    string dueDate;
    
public:
    AVN(const AVNMessage& msg) 
    {
        avnID = msg.avnID;
        flightNumber = msg.flightNumber;
        airlineName = msg.airlineName;
        flightType = msg.flightType;
        recordedSpeed = msg.recordedSpeed;
        allowedSpeedLow = msg.allowedSpeedLow;
        allowedSpeedHigh = msg.allowedSpeedHigh;
        issuanceDateTime = msg.issuanceDateTime;
        baseAmount = msg.baseAmount;
        serviceFee = msg.serviceFee;
        totalAmount = msg.totalAmount;
        paymentStatus = msg.paymentStatus;
        dueDate = msg.dueDate;
    }

    // Getters
    string getAVNID() const { return avnID; }
    string getFlightNumber() const { return flightNumber; }
    string getAirlineName() const { return airlineName; }
    string getFlightType() const { return flightType; }
    int getRecordedSpeed() const { return recordedSpeed; }
    int getAllowedSpeedLow() const { return allowedSpeedLow; }
    int getAllowedSpeedHigh() const { return allowedSpeedHigh; }
    string getIssuanceDateTime() const { return issuanceDateTime; }
    double getBaseAmount() const { return baseAmount; }
    double getServiceFee() const { return serviceFee; }
    double getTotalAmount() const { return totalAmount; }
    string getPaymentStatus() const { return paymentStatus; }
    string getDueDate() const { return dueDate; }

    // Setters
    void setPaymentStatus(string status) 
    {
        paymentStatus = status;
    }

    void display() {
        cout << "===== AVN DETAILS =====" << endl;
        cout << "AVN ID: " << avnID << endl;
        cout << "Airline: " << airlineName << endl;
        cout << "Flight: " << flightNumber << endl;
        cout << "Flight Type: " << flightType << endl;
        cout << "Speed Recorded: " << recordedSpeed << " km/h" << endl;
        cout << "Speed Limit: " << allowedSpeedLow << " km/h to " << allowedSpeedHigh << " km/h" << endl;
        cout << "Issued On: " << issuanceDateTime << endl;
        cout << "Due Date: " << dueDate << endl;
        cout << "Base Amount: PKR " << baseAmount << endl;
        cout << "Service Fee: PKR " << serviceFee << endl;
        cout << "Total Amount: PKR " << totalAmount << endl;
        cout << "Payment Status: " << paymentStatus << endl;
        cout << "======================" << endl;
    }
};

class StripePay 
{
public:
    bool processPayment(string avnID, string flightNumber, string flightType, double amount) 
    {
        cout << "\n===== STRIPE PAYMENT PROCESSING =====" << endl;
        cout << "Processing payment for AVN: " << avnID << endl;
        cout << "Flight: " << flightNumber << endl;
        cout << "Flight Type: " << flightType << endl;
        cout << "Amount: PKR " << amount << endl;
        
        // Simulate payment process
        cout << "Enter amount to pay (PKR): ";
        double paidAmount;
        cin >> paidAmount;
        
        if (paidAmount >= amount) 
        {
            cout << green << "Payment Successful!" << default_text << endl;
            
            // Send status update to ATC process
            sendStatusUpdate(avnID.c_str(), "paid", flightNumber.c_str());
            
            return true;
        } 
        else 
        {
            cout << red << "Payment Failed! Insufficient amount." << default_text << endl;
            return false;
        }
    }
};
    
class AirlinePortal 
{
private:
    map<string, vector<AVN*>> airlineAVNs;
    StripePay* stripePay;

public:
    AirlinePortal() 
    {
        stripePay = new StripePay();
    }

    ~AirlinePortal() 
    {
        for (auto& pair : airlineAVNs) 
        {
            for (auto* avn : pair.second) 
            {
                delete avn;
            }
        }
        delete stripePay;
    }

    void addAVN(AVN* avn) 
    {
        string airline = avn->getAirlineName();
        airlineAVNs[airline].push_back(avn);
        cout << cyan << "AVN " << avn->getAVNID() << " added to " << airline << default_text << endl;
    }

    void displayAVNs(string airlineName) 
    {
        cout << "\n============== Airline Portal: " << airlineName << " ===============\n";
        if (airlineAVNs.find(airlineName) != airlineAVNs.end()) 
        {
            if (airlineAVNs[airlineName].empty()) 
            {
                cout << "No violations found for " << airlineName << endl;
            } 
            else 
            {
                cout << "Active and Past AVNs:" << endl;
                for (auto avn : airlineAVNs[airlineName]) 
                {
                    cout << "AVN ID: " << avn->getAVNID() << endl;
                    cout << "Flight: " << avn->getFlightNumber() << endl;
                    cout << "Payment Status: " << avn->getPaymentStatus() << endl;
                    cout << "Total Amount: PKR " << avn->getTotalAmount() << endl;
                    cout << "Due Date: " << avn->getDueDate() << endl;
                    cout << "----------------------------" << endl;
                }
            }
        } 
        else 
        {
            cout << "No airlines found with name: " << airlineName << endl;
        }
        cout << "\n==========================================================\n";
    }

    void updateAVNStatus(string avnID, string status) 
    {
        for (auto& pair : airlineAVNs) 
        {
            for (auto avn : pair.second) 
            {
                if (avn->getAVNID() == avnID) 
                {
                    avn->setPaymentStatus(status);
                    cout << green << "Airline Portal: Updated AVN " << avnID 
                            << " status to " << status << default_text << endl;
                    
                    sendStatusUpdate(avnID.c_str(), status.c_str(), avn->getFlightNumber().c_str());
                    
                    return;
                }
            }
        }
    }

    bool payAVN(string avnID) 
    {
        for (auto& pair : airlineAVNs) 
        {
            for (auto avn : pair.second) 
            {
                if (avn->getAVNID() == avnID) 
                {
                    if (avn->getPaymentStatus() == "unpaid") 
                    {
                        // Process payment through StripePay
                        bool success = stripePay->processPayment(
                            avn->getAVNID(),
                            avn->getFlightNumber(),
                            avn->getFlightType(),
                            avn->getTotalAmount()
                        );
                        
                        if (success) 
                        {
                            avn->setPaymentStatus("paid");
                            return true;
                        }
                    } 
                    else 
                    {
                        cout << yellow << "This AVN is already " << avn->getPaymentStatus() << default_text << endl;
                    }
                    return false;
                }
            }
        }
        
        cout << red << "AVN not found with ID: " << avnID << default_text << endl;
        return false;
    }

    void displayAvailableAirlines() 
    {
        cout << "\n===== Available Airlines =====\n";
        if (airlineAVNs.empty()) 
        {
            cout << "No airlines with AVNs" << endl;
        } 
        else 
        {
            for (const auto& pair : airlineAVNs) 
            {
                cout << "- " << pair.first << " (" << pair.second.size() << " AVNs)" << endl;
            }
        }
        cout << "===========================\n";
    }
    
    AVN* getAVN(string avnID) 
    {
        for (auto& pair : airlineAVNs) 
        {
            for (auto avn : pair.second) 
            {
                if (avn->getAVNID() == avnID) 
                {
                    return avn;
                }
            }
        }
        return nullptr;
    }
};

// Global AirlinePortal instance
AirlinePortal* airlinePortal = nullptr;

// Function to send status updates to ATC process
void sendStatusUpdate(const char* avnID, const char* status, const char* flightNumber) 
{
    // Open pipe for writing
    int pipe_fd = open(AIRLINE_TO_ATC_PIPE, O_WRONLY | O_NONBLOCK);
    if (pipe_fd == -1) 
    {
        perror("Error opening status pipe for writing");
        return;
    }

    // Prepare message
    StatusMessage msg;
    strncpy(msg.avnID, avnID, sizeof(msg.avnID) - 1);
    strncpy(msg.status, status, sizeof(msg.status) - 1);
    strncpy(msg.flightNumber, flightNumber, sizeof(msg.flightNumber) - 1);

    // Write to pipe
    ssize_t bytes_written = write(pipe_fd, &msg, sizeof(msg));
    if (bytes_written == -1) 
    {
        perror("Error writing status to pipe");
    } else 
    {
        cout << green << "Sent status update for AVN " << avnID << " to ATC process" << default_text << endl;
    }

    close(pipe_fd);
}

// Thread function to receive AVNs from ATC process
void* receiveAVNs(void* arg) 
{
    AirlinePortal* portal = (AirlinePortal*)arg;
    
    // Open pipe for reading
    int pipe_fd = open(ATC_TO_AIRLINE_PIPE, O_RDONLY  | O_NONBLOCK);
    if (pipe_fd == -1) 
    {
        perror("Error opening pipe for reading");
        return NULL;
    }

    cout << green << "AVN receiver started" << default_text << endl;
    
    AVNMessage msg;
    while (1) 
    {
        ssize_t bytes_read = read(pipe_fd, &msg, sizeof(msg));
        if (bytes_read > 0) 
        {
            cout << yellow << "Received AVN " << msg.avnID << " from ATC process" << default_text << endl;
            
            // Create AVN object and add to portal
            AVN* avn = new AVN(msg);
            portal->addAVN(avn);
            
            // Notify the user about the new AVN
            cout << "\n" << cyan << "!!! New AVN received !!!" << default_text << endl;
            avn->display();
        }
    }
    
    close(pipe_fd);
    return NULL;
}

int main() 
{
    // Register signal handler for cleanup
    signal(SIGINT, cleanupPipes);
    
    cout << green << "Airline Process started" << default_text << endl;
    cout << "Connecting to ATC process..." << endl;

    // Initialize AirlinePortal
    airlinePortal = new AirlinePortal();

    // Open pipe for reading (will block until ATC process opens it for writing)
    int pipe_fd = open(ATC_TO_AIRLINE_PIPE, O_RDONLY);
    if (pipe_fd == -1) 
    {
        perror("Error opening pipe for reading");
        return 1;
    }
    close(pipe_fd);

    cout << green << "Connected to ATC process!" << default_text << endl;

    // Start thread to receive AVNs
    pthread_t receiverThread;
    if (pthread_create(&receiverThread, NULL, receiveAVNs, airlinePortal) != 0) 
    {
        perror("Error creating receiver thread");
        return 1;
    }

    // Main loop for airline operations
    cout << yellow << "Airline Process running. Enter commands:" << default_text << endl;
    cout << "1. list - List all airlines with AVNs" << endl;
    cout << "2. avns <airline> - Show AVNs for an airline" << endl;
    cout << "3. details <avnID> - Show details of an AVN" << endl;
    cout << "4. pay <avnID> - Pay an AVN" << endl;
    cout << "5. exit - Exit the program" << endl;

    string command;
    while (true) 
    {
        cout << "> ";
        getline(cin, command);
        
        if (command == "list") 
        {
            airlinePortal->displayAvailableAirlines();
        }
        else if (command.find("avns") == 0) 
        {
            string airline = command.substr(5);
            if (!airline.empty()) 
            {
                airlinePortal->displayAVNs(airline);
            } 
            else 
            {
                cout << red << "Please specify an airline name" << default_text << endl;
            }
        }
        else if (command.find("details") == 0) 
        {
            string avnID = command.substr(8);
            if (!avnID.empty()) 
            {
                AVN* avn = airlinePortal->getAVN(avnID);
                if (avn) 
                {
                    avn->display();
                } 
                else 
                {
                    cout << red << "AVN not found" << default_text << endl;
                }
            } 
            else 
            {
                cout << red << "Please specify an AVN ID" << default_text << endl;
            }
        }
        else if (command.find("pay") == 0) 
        {
            string avnID = command.substr(4);
            if (!avnID.empty()) 
            {
                airlinePortal->payAVN(avnID);
            }
            else 
            {
            
                cout << red << "Please specify an AVN ID" << default_text << endl;
            }
        }
        else if (command == "exit") 
        {
            break;
        }
        else 
        {
            cout << red << "Unknown command" << default_text << endl;
        }
    }

    // Clean up
    pthread_cancel(receiverThread);
    pthread_join(receiverThread, NULL);
    delete airlinePortal;
    
    return 0;
}