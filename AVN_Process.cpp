#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <fstream>
#include <signal.h>
#include "AVN.hpp"
#include "PipeAVNGenerator.hpp"  // Include the new header
#include "ATCSController.hpp"    // Now we can include the full ATCSController
#include "Airline.hpp"
#include "Colors.hpp"

using namespace std;

bool pipeInitialized = false;

// Implementation of PipeAVNGenerator methods
PipeAVNGenerator::PipeAVNGenerator(ATCSController* controller) : atcController(controller) {}

PipeAVNGenerator::~PipeAVNGenerator() {
    for(auto avn : issuedAVNs) {
        delete avn;
    }
}

AVN* PipeAVNGenerator::createAVN(Flight* flight, int recordedSpeed, int allowedHigh, int allowedLow) {
    string actualAirlineName = "Unknown";
    if(flight->getParentAirline() != NULL) {
        actualAirlineName = flight->getParentAirline()->getName();
    }
    
    AVN* newAVN = new AVN(flight->getID(), actualAirlineName, flight->getFlightType(), recordedSpeed, allowedLow, allowedHigh);
                          
    issuedAVNs.push_back(newAVN);
    cout << cyan << "AVN Generated: " << newAVN->getAVNID() << " for " << flight->getID() << default_text << endl;
    
    // Send the AVN to the airline process through pipe
    cout << red << "OMG! SENDING AVN TO THE PIPE WAAAAAT!" << endl;
    sendAVNToPipe(newAVN);
    
    return newAVN;
}

void PipeAVNGenerator::updatePaymentStatus(string avnID, string status) {
    for(auto avn : issuedAVNs) {
        if(avn->getAVNID() == avnID) {
            avn->setPaymentStatus(status);
            cout << green << "Payment Status updated for AVN " << avnID << " to " << status << default_text << endl;
            
            // If status is paid, clear the violation
            if(status == "paid") {
                for(auto flight : issuedAVNs) {
                    if(flight->getAVNID() == avnID) {
                        if(atcController != nullptr) {
                            atcController->clearViolation(flight->getFlightNumber());
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
}

vector<AVN*> PipeAVNGenerator::getIssuedAVNs() {
    return issuedAVNs;
}

void PipeAVNGenerator::setATCController(ATCSController* controller) {
    atcController = controller;
}

// Implementation of pipe functions
void cleanupPipes(int signal) {
    cout << "Cleaning up pipes..." << endl;
    unlink(ATC_TO_AIRLINE_PIPE);
    unlink(AIRLINE_TO_ATC_PIPE);
    exit(0);
}

// Function to send AVN data through pipe
void sendAVNToPipe(AVN* avn) {
    if (!pipeInitialized) {
        cout << red << "Error: Pipe not initialized yet!" << default_text << endl;
        return;
    }

    // Open the pipe for writing
    int pipe_fd = open(ATC_TO_AIRLINE_PIPE, O_WRONLY);
    if (pipe_fd == -1) {
        perror("Error opening pipe for writing");
        return;
    }

    // Prepare message
    AVNMessage msg;
    strncpy(msg.avnID, avn->getAVNID().c_str(), sizeof(msg.avnID) - 1);
    strncpy(msg.flightNumber, avn->getFlightNumber().c_str(), sizeof(msg.flightNumber) - 1);
    strncpy(msg.airlineName, avn->getAirlineName().c_str(), sizeof(msg.airlineName) - 1);
    strncpy(msg.flightType, avn->getflightType().c_str(), sizeof(msg.flightType) - 1);
    msg.recordedSpeed = avn->getRecordedSpeed();
    msg.allowedSpeedLow = avn->getAllowedSpeedLow();
    msg.allowedSpeedHigh = avn->getAllowedSpeedHigh();
    strncpy(msg.issuanceDateTime, avn->getIssuanceDateTime().c_str(), sizeof(msg.issuanceDateTime) - 1);
    msg.baseAmount = avn->getBaseAmount();
    msg.serviceFee = avn->getServiceFee();
    msg.totalAmount = avn->getTotalAmount();
    strncpy(msg.paymentStatus, avn->getPaymentStatus().c_str(), sizeof(msg.paymentStatus) - 1);
    strncpy(msg.dueDate, avn->getDueDate().c_str(), sizeof(msg.dueDate) - 1);

    // Write to pipe
    ssize_t bytes_written = write(pipe_fd, &msg, sizeof(msg));
    if (bytes_written == -1) {
        perror("Error writing to pipe");
    } else {
        cout << green << "Sent AVN " << avn->getAVNID() << " to airline process" << default_text << endl;
    }

    close(pipe_fd);
}

// Thread function to receive status updates from airline process
void* receiveStatusUpdates(void* arg) {
    ATCSController* controller = (ATCSController*)arg;
    
    // Open pipe for reading
    int pipe_fd = open(AIRLINE_TO_ATC_PIPE, O_RDONLY);
    if (pipe_fd == -1) {
        perror("Error opening status pipe for reading");
        return NULL;
    }

    cout << green << "Status update listener started" << default_text << endl;
    
    StatusMessage msg;
    while (1) {
        ssize_t bytes_read = read(pipe_fd, &msg, sizeof(msg));
        if (bytes_read > 0) {
            cout << yellow << "Received status update for AVN " << msg.avnID << ": " << msg.status << default_text << endl;
            
            // Update AVN status
            PipeAVNGenerator* generator = (PipeAVNGenerator*)controller->getAVNGenerator();
            generator->updatePaymentStatus(msg.avnID, msg.status);
            
            // If status is "paid", clear the violation
            if (strcmp(msg.status, "paid") == 0) {
                controller->clearViolation(msg.flightNumber);
            }
        }
    }
    
    close(pipe_fd);
    return NULL;
}