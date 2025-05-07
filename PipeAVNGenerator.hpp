#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "AVN.hpp"
#include "AVNGenerator.hpp"

// Forward declaration
class ATCSController;
class Flight;

// Define message structures
struct AVNMessage {
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

struct StatusMessage {
    char avnID[50];
    char status[20];
    char flightNumber[20];
};

// Define pipe names as constants
#define ATC_TO_AIRLINE_PIPE "atc_to_airline_pipe"
#define AIRLINE_TO_ATC_PIPE "airline_to_atc_pipe"
#define SIM_TO_AVN_PIPE "sim_to_avn_pipe"


// Pipe AVN Generator
class PipeAVNGenerator : public AVNGenerator {
private:
    std::vector<AVN*> issuedAVNs;
    ATCSController* atcController;
    
public:
    PipeAVNGenerator(ATCSController* controller);
    ~PipeAVNGenerator();
    
    AVN* createAVN(Flight* flight, int recordedSpeed, int allowedHigh, int allowedLow);
    void updatePaymentStatus(std::string avnID, std::string status);
    std::vector<AVN*> getIssuedAVNs();
    void setATCController(ATCSController* controller);
};

// Function declarations
void sendAVNToPipe(AVN* avn);
void* receiveStatusUpdates(void* arg);
void cleanupPipes(int signal);
void initializePipes();