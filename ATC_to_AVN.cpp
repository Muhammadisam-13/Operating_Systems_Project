#include "ATC_to_AVN.hpp"
#include "Colors.hpp"
#include <cstring>
#include <signal.h>

using namespace std;

int fd_sim_to_avn = -1;
bool sim_avn_pipe_initialized = false;

bool fileExists(const char* filename) 
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void initializeSimToAVNPipe() {
    std::cout << "Starting Simulation to AVN pipe initialization..." << std::endl;
    
    // Close any existing file descriptor
    if (fd_sim_to_avn != -1) {
        close(fd_sim_to_avn);
        fd_sim_to_avn = -1;
    }
    
    // Remove any existing pipe
    if (access(SIM_TO_AVN_PIPE, F_OK) != -1) {
        if (unlink(SIM_TO_AVN_PIPE) != 0) {
            std::cout << "Warning: Failed to remove existing " << SIM_TO_AVN_PIPE 
                     << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Removed existing " << SIM_TO_AVN_PIPE << std::endl;
        }
    }

    if (fileExists(SIM_TO_AVN_PIPE)) {
        if (unlink(SIM_TO_AVN_PIPE) != 0) {
            std::cout << "Warning: Failed to remove existing " << SIM_TO_AVN_PIPE 
                     << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Removed existing " << SIM_TO_AVN_PIPE << std::endl;
        }
    }
    
    // Brief pause to ensure system resources are released
    usleep(200000);  // 200ms
    
    std::cout << "Creating new pipe for simulation to AVN communication..." << std::endl;
    
    // Create the pipe
    if (mkfifo(SIM_TO_AVN_PIPE, 0666) != 0) {
        std::cout << "Error creating " << SIM_TO_AVN_PIPE << ": " 
                 << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        
        // If the error is "file exists", try to continue anyway
        if (errno != EEXIST) {
            return;
        } else {
            std::cout << "Pipe already exists, continuing..." << std::endl;
        }
    }
    
    // Open the pipe for writing
    fd_sim_to_avn = open(SIM_TO_AVN_PIPE, O_WRONLY);
    if (fd_sim_to_avn == -1) {
        std::cout << "Failed to open " << SIM_TO_AVN_PIPE << " for writing: " 
                 << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        unlink(SIM_TO_AVN_PIPE);
        return;
    }
    
    std::cout << green << "Simulation to AVN pipe successfully initialized" << default_text << std::endl;
    sim_avn_pipe_initialized = true;
    
    // Set up signal handler
    signal(SIGINT, cleanupSimToAVNPipe);
}

void cleanupSimToAVNPipe(int signal) {
    std::cout << "Cleaning up Simulation to AVN pipe..." << std::endl;
    
    if (fd_sim_to_avn != -1) {
        close(fd_sim_to_avn);
        fd_sim_to_avn = -1;
    }
    
    if (unlink(SIM_TO_AVN_PIPE) == 0) {
        std::cout << "Successfully removed " << SIM_TO_AVN_PIPE << std::endl;
    } else if (errno != ENOENT) {
        std::cout << "Failed to remove " << SIM_TO_AVN_PIPE << ": " 
                 << strerror(errno) << std::endl;
    }
    
    sim_avn_pipe_initialized = false;
    
    // Only exit if called as a signal handler
    if (signal != 0) {
        exit(signal);
    }
}

bool sendViolationToAVNProcess(
    const char* flightID, 
    const char* airlineName, 
    const char* flightType,
    int recordedSpeed, 
    int allowedSpeedLow, 
    int allowedSpeedHigh,
    const char* flightPhase
) {
    if (!sim_avn_pipe_initialized || fd_sim_to_avn == -1) {
        std::cout << "Cannot send violation: Pipe not initialized or opened" << std::endl;
        return false;
    }
    
    // Prepare message
    SimViolationMessage msg;
    
    strncpy(msg.flightID, flightID, sizeof(msg.flightID) - 1);
    msg.flightID[sizeof(msg.flightID) - 1] = '\0';
    
    strncpy(msg.airlineName, airlineName, sizeof(msg.airlineName) - 1);
    msg.airlineName[sizeof(msg.airlineName) - 1] = '\0';
    
    strncpy(msg.flightType, flightType, sizeof(msg.flightType) - 1);
    msg.flightType[sizeof(msg.flightType) - 1] = '\0';
    
    msg.recordedSpeed = recordedSpeed;
    msg.allowedSpeedLow = allowedSpeedLow;
    msg.allowedSpeedHigh = allowedSpeedHigh;
    
    strncpy(msg.flightPhase, flightPhase, sizeof(msg.flightPhase) - 1);
    msg.flightPhase[sizeof(msg.flightPhase) - 1] = '\0';
    
    // Write to pipe with retry logic
    int retry_count = 0;
    bool write_success = false;
    
    while (retry_count < 3 && !write_success) {
        ssize_t bytes_written = write(fd_sim_to_avn, &msg, sizeof(msg));
        
        if (bytes_written == sizeof(msg)) {
            std::cout << cyan << "Successfully sent violation for flight " << flightID 
                      << " to AVN Process" << default_text << std::endl;
            write_success = true;
        } else if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Pipe full, retrying in 100ms..." << std::endl;
                usleep(100000); // 100ms
            } else {
                perror("Failed to write violation to pipe");
                break; // Exit loop on permanent error
            }
        } else {
            std::cout << "Warning: Partial write of violation to pipe (" << bytes_written 
                      << " of " << sizeof(msg) << " bytes)" << std::endl;
            break;
        }
        
        retry_count++;
    }
    
    if (!write_success) {
        std::cout << red << "Failed to send violation after " << retry_count << " attempts" 
                  << default_text << std::endl;
    }
    
    return write_success;
}

void* receiveViolationsFromSim(void* arg) {
    // Cast the argument to AVNGenerator pointer (or PipeAVNGenerator)
    // We'll adapt this in the AVN_Process.cpp implementation
    void* generator = arg;
    
    // Open pipe for reading
    int pipe_fd = open(SIM_TO_AVN_PIPE, O_RDONLY | O_NONBLOCK);
    if (pipe_fd == -1) {
        perror("Error opening SimToAVN pipe for reading");
        return NULL;
    }
    
    std::cout << green << "Violation receiver started in AVN Process" << default_text << std::endl;
    
    SimViolationMessage msg;
    while (1) {
        ssize_t bytes_read = read(pipe_fd, &msg, sizeof(msg));
        if (bytes_read > 0) {
            std::cout << yellow << "Received violation for flight " << msg.flightID 
                      << " from Simulation process" << default_text << std::endl;
            
            // Process the violation message
            // We'll implement this in AVN_Process.cpp
            // This will create an AVN and send it to the airline
            
            // For now, just log it
            std::cout << "Flight ID: " << msg.flightID << std::endl;
            std::cout << "Airline: " << msg.airlineName << std::endl;
            std::cout << "Speed: " << msg.recordedSpeed << " (limit: " 
                      << msg.allowedSpeedLow << "-" << msg.allowedSpeedHigh << ")" << std::endl;
            std::cout << "Phase: " << msg.flightPhase << std::endl;
        } else if (bytes_read < 0 && errno != EAGAIN) {
            perror("Error reading from SimToAVN pipe");
            sleep(1);
        }
        
        usleep(100000); // 100ms delay
    }
    
    close(pipe_fd);
    return NULL;
}