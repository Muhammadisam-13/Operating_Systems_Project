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

int fd_atc_to_airline = -1;
int fd_airline_to_atc = -1;
pthread_t receive_thread;
bool pipes_initialized = false;

bool fileExists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void initializePipes() 
{
    std::cout << "Starting pipe initialization..." << std::endl;
    
    // First, ensure any existing file descriptors are closed
    if (fd_atc_to_airline != -1) {
        close(fd_atc_to_airline);
        fd_atc_to_airline = -1;
    }
    
    if (fd_airline_to_atc != -1) {
        close(fd_airline_to_atc);
        fd_airline_to_atc = -1;
    }
    
    // Remove any existing pipes with better error reporting
    if (fileExists(ATC_TO_AIRLINE_PIPE)) {
        if (unlink(ATC_TO_AIRLINE_PIPE) != 0) {
            std::cout << "Warning: Failed to remove existing " << ATC_TO_AIRLINE_PIPE 
                     << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Removed existing " << ATC_TO_AIRLINE_PIPE << std::endl;
        }
    }
    
    if (fileExists(AIRLINE_TO_ATC_PIPE)) {
        if (unlink(AIRLINE_TO_ATC_PIPE) != 0) {
            std::cout << "Warning: Failed to remove existing " << AIRLINE_TO_ATC_PIPE 
                     << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Removed existing " << AIRLINE_TO_ATC_PIPE << std::endl;
        }
    }
    
    // Sleep briefly to ensure system has released resources
    usleep(200000);  // 200ms
    
    std::cout << "Creating new pipes for communication..." << std::endl;
    
    // Create ATC_TO_AIRLINE_PIPE with explicit error checking
    if (mkfifo(ATC_TO_AIRLINE_PIPE, 0666) != 0) {
        std::cout << "Error creating " << ATC_TO_AIRLINE_PIPE << ": " 
                 << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        
        // If the error is "file exists", try to continue anyway
        if (errno != EEXIST) {
            return;
        } else {
            std::cout << "Pipe already exists, continuing..." << std::endl;
        }
    }
    
    // Create AIRLINE_TO_ATC_PIPE with explicit error checking
    if (mkfifo(AIRLINE_TO_ATC_PIPE, 0666) != 0) {
        std::cout << "Error creating " << AIRLINE_TO_ATC_PIPE << ": " 
                 << strerror(errno) << " (errno: " << errno << ")" << std::endl;
                 
        // If the error is "file exists", try to continue anyway
        if (errno != EEXIST) {
            unlink(ATC_TO_AIRLINE_PIPE); // Clean up first pipe
            return;
        } else {
            std::cout << "Pipe already exists, continuing..." << std::endl;
        }
    }
    
    std::cout << "Pipes created, now attempting to open them..." << std::endl;
    
    // CRITICAL FIX: Use blocking mode for first open to prevent "No such device" error
    // Opening read pipe first, then write pipe - this ordering is important
    std::cout << "Opening read pipe in blocking mode..." << std::endl;
    fd_airline_to_atc = open(AIRLINE_TO_ATC_PIPE, O_RDONLY | O_NONBLOCK);
    if (fd_airline_to_atc == -1) {
        std::cout << "Failed to open " << AIRLINE_TO_ATC_PIPE << " for reading: " 
                 << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        unlink(ATC_TO_AIRLINE_PIPE);
        unlink(AIRLINE_TO_ATC_PIPE);
        return;
    }
    
    cout << "Read Pipe created!" << endl;
    // Then wait for the write pipe to be available 
    std::cout << "Opening write pipe with retries..." << std::endl;

        fd_atc_to_airline = open(ATC_TO_AIRLINE_PIPE, O_WRONLY);
        cout << fd_atc_to_airline << endl;
        std::cout << "Waiting for airline process to connect..." << std::endl;
        //usleep(1000000); // 1 second delay between attempts
        //retry_count++;
    //}
    
    cout << "Write Pipe Created!" << endl;
    if (fd_atc_to_airline == -1) 
    {
        close(fd_airline_to_atc);
        unlink(ATC_TO_AIRLINE_PIPE);
        unlink(AIRLINE_TO_ATC_PIPE);
        return;
    }
    
    // Set to non-blocking mode for later operations
    int flags = fcntl(fd_atc_to_airline, F_GETFL, 0);
    fcntl(fd_atc_to_airline, F_SETFL, flags | O_NONBLOCK);

    pipes_initialized = true;
    std::cout << "Pipes successfully initialized" << std::endl;
    
    // Set up signal handler for cleanup
    signal(SIGINT, cleanupPipes);
}


// Implementation of PipeAVNGenerator methods
PipeAVNGenerator::PipeAVNGenerator(ATCSController* controller) : atcController(controller) 
{
    std::cout << "Creating PipeAVNGenerator..." << std::endl;
    
    // Try to initialize pipes but handle failures gracefully
    initializePipes();
    
    if (pipes_initialized) 
    {
        if (pthread_create(&receive_thread, NULL, receiveStatusUpdates, this) != 0) 
        {
            perror("Failed to create receive thread");
        } else {
            std::cout << "Status update receiver thread started" << std::endl;
        }
    } else {
        std::cout << "Warning: Starting PipeAVNGenerator without initialized pipes" << std::endl;
        std::cout << "AVNs will be created but not sent to the airline process" << std::endl;
    }
}

PipeAVNGenerator::~PipeAVNGenerator() 
{
    if (pipes_initialized) 
    {
        pthread_cancel(receive_thread);
        pthread_join(receive_thread, NULL);
        
        cleanupPipes(0);
    }
}

AVN* PipeAVNGenerator::createAVN(Flight* flight, int recordedSpeed, int allowedHigh, int allowedLow) {

    std::cout << "Creating AVN for flight " << flight->getID() << std::endl;

    // Try initializing pipes if not already done
    if (!pipes_initialized) 
    {
        std::cout << "Pipes not initialized, attempting to initialize..." << std::endl;
        initializePipes();
        
        if (!pipes_initialized) 
        {
            std::cout << "Pipe initialization failed, continuing without pipe communication" << std::endl;
        }
    }
    
    // Create AVN using parent class method
    AVN* newAVN = AVNGenerator::createAVN(flight, recordedSpeed, allowedHigh, allowedLow);
    std::cout << "Created AVN " << newAVN->getAVNID() << " for flight " << flight->getID() << std::endl;
    
    // Store the AVN
    issuedAVNs.push_back(newAVN);
    
    // Send AVN to airline through pipe if pipes are initialized
    if (pipes_initialized) {
        sendAVNToPipe(newAVN);
    } else {
        std::cout << "Warning: AVN created but not sent through pipe" << std::endl;
    }
    
    return newAVN;
}

void PipeAVNGenerator::updatePaymentStatus(string avnID, string status) {
    // Flag to track if the AVN was found
    bool avnFound = false;
        
    for(auto avn : issuedAVNs) {
        if(avn->getAVNID() == avnID) {
            avnFound = true;
            avn->setPaymentStatus(status);
            cout << green << "Payment Status updated for AVN " << avnID << " to " << status << default_text << endl;
            
            // If status is paid, clear the violation
            if(status == "paid") {
                if(atcController != nullptr) {
                    atcController->clearViolation(avn->getFlightNumber());
                } else {
                    cout << "Warning: Cannot clear violation - ATC Controller is null" << endl;
                }
            }
            break;
        }
    }
    
    if (!avnFound) {
        cout << "Warning: Attempted to update payment status for unknown AVN: " << avnID << endl;
    }
}

vector<AVN*> PipeAVNGenerator::getIssuedAVNs() {
    return issuedAVNs;
}

void PipeAVNGenerator::setATCController(ATCSController* controller) {
    atcController = controller;
}

// Implementation of pipe functions
void cleanupPipes(int signal) 
{
    std::cout << "Cleaning up pipes..." << std::endl;
    
    if (fd_atc_to_airline != -1) {
        close(fd_atc_to_airline);
        fd_atc_to_airline = -1;
    }
    
    if (fd_airline_to_atc != -1) {
        close(fd_airline_to_atc);
        fd_airline_to_atc = -1;
    }
    
    // Report unlink operations
    if (unlink(ATC_TO_AIRLINE_PIPE) == 0) {
        std::cout << "Successfully removed " << ATC_TO_AIRLINE_PIPE << std::endl;
    } else if (errno != ENOENT) { // Only report error if file exists but can't be removed
        std::cout << "Failed to remove " << ATC_TO_AIRLINE_PIPE << ": " 
                 << strerror(errno) << std::endl;
    }
    
    if (unlink(AIRLINE_TO_ATC_PIPE) == 0) {
        std::cout << "Successfully removed " << AIRLINE_TO_ATC_PIPE << std::endl;
    } else if (errno != ENOENT) { // Only report error if file exists but can't be removed
        std::cout << "Failed to remove " << AIRLINE_TO_ATC_PIPE << ": " 
                 << strerror(errno) << std::endl;
    }
    
    pipes_initialized = false;
    
    // Only exit if called as a signal handler
    if (signal != 0) {
        exit(signal);
    }
}

// Function to send AVN data through pipe
void sendAVNToPipe(AVN* avn) 
{
    if (!pipes_initialized || fd_atc_to_airline == -1) 
    {
        cout << "Cannot send AVN: Pipes not initialized or opened" << endl;
        return;
    }

    // Prepare message
    AVNMessage msg;
    strncpy(msg.avnID, avn->getAVNID().c_str(), sizeof(msg.avnID) - 1);
    msg.avnID[sizeof(msg.avnID) - 1] = '\0'; // Ensure null-termination
    
    strncpy(msg.flightNumber, avn->getFlightNumber().c_str(), sizeof(msg.flightNumber) - 1);
    msg.flightNumber[sizeof(msg.flightNumber) - 1] = '\0';
    
    strncpy(msg.airlineName, avn->getAirlineName().c_str(), sizeof(msg.airlineName) - 1);
    msg.airlineName[sizeof(msg.airlineName) - 1] = '\0';
    
    strncpy(msg.flightType, avn->getflightType().c_str(), sizeof(msg.flightType) - 1);
    msg.flightType[sizeof(msg.flightType) - 1] = '\0';
    
    msg.recordedSpeed = avn->getRecordedSpeed();
    msg.allowedSpeedLow = avn->getAllowedSpeedLow();
    msg.allowedSpeedHigh = avn->getAllowedSpeedHigh();
    
    strncpy(msg.issuanceDateTime, avn->getIssuanceDateTime().c_str(), sizeof(msg.issuanceDateTime) - 1);
    msg.issuanceDateTime[sizeof(msg.issuanceDateTime) - 1] = '\0';
    
    msg.baseAmount = avn->getBaseAmount();
    msg.serviceFee = avn->getServiceFee();
    msg.totalAmount = avn->getTotalAmount();
    
    strncpy(msg.paymentStatus, avn->getPaymentStatus().c_str(), sizeof(msg.paymentStatus) - 1);
    msg.paymentStatus[sizeof(msg.paymentStatus) - 1] = '\0';
    
    strncpy(msg.dueDate, avn->getDueDate().c_str(), sizeof(msg.dueDate) - 1);
    msg.dueDate[sizeof(msg.dueDate) - 1] = '\0';

    // Write to pipe with retry logic
    int retry_count = 0;
    bool write_success = false;
    
    while (retry_count < 3 && !write_success) {
        ssize_t bytes_written = write(fd_atc_to_airline, &msg, sizeof(msg));
        
        if (bytes_written == sizeof(msg)) {
            std::cout << "Successfully sent AVN " << avn->getAVNID() << " to airline" << std::endl;
            write_success = true;
        } else if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Pipe full, retrying in 100ms..." << std::endl;
                usleep(100000); // 100ms
            } else {
                perror("Failed to write AVN to pipe");
                break; // Exit loop on permanent error
            }
        } else {
            std::cout << "Warning: Partial write of AVN to pipe (" << bytes_written 
                      << " of " << sizeof(msg) << " bytes)" << std::endl;
            break;
        }
        
        retry_count++;
    }
    
    if (!write_success) {
        std::cout << "Failed to send AVN after " << retry_count << " attempts" << std::endl;
    }
}

// Thread function to receive status updates from airline process
void* receiveStatusUpdates(void* arg) 
{
    PipeAVNGenerator* generator = static_cast<PipeAVNGenerator*>(arg);
    
    while (true) {
        if (!pipes_initialized || fd_airline_to_atc == -1) {
            sleep(1);
            continue;
        }
        
        StatusMessage message;
        ssize_t bytes_read = read(fd_airline_to_atc, &message, sizeof(message));
        
        if (bytes_read > 0) {
            // Ensure null-termination
            message.avnID[sizeof(message.avnID) - 1] = '\0';
            message.status[sizeof(message.status) - 1] = '\0';
            message.flightNumber[sizeof(message.flightNumber) - 1] = '\0';
            
            std::string avnID(message.avnID);
            std::string status(message.status);
            std::string flightNumber(message.flightNumber);
            
            std::cout << "Received status update for AVN " << avnID << ": " << status << std::endl;
            
            // Update payment status
            generator->updatePaymentStatus(avnID, status);
        } else if (bytes_read < 0 && errno != EAGAIN) {
            perror("Error reading from airline pipe");
            sleep(1);
        } else {
            // No data available or EAGAIN error
            usleep(100000); // Sleep for 100ms
        }
    }
    
    return NULL;
}