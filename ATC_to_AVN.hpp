#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <fstream>
#include <signal.h>
#include <iostream>
#include <filesystem>
#include "AVN.hpp"

// Define message structure for simulation to AVN process communication
struct SimViolationMessage {
    char flightID[50];
    char airlineName[50];
    char flightType[20];
    int recordedSpeed;
    int allowedSpeedLow;
    int allowedSpeedHigh;
    char flightPhase[30];
};

// Define pipe name as constant
#define SIM_TO_AVN_PIPE "sim_to_avn_pipe"

// Function declarations
void initializeSimToAVNPipe();
void cleanupSimToAVNPipe(int signal);
bool sendViolationToAVNProcess(
    const char* flightID, 
    const char* airlineName, 
    const char* flightType,
    int recordedSpeed, 
    int allowedSpeedLow, 
    int allowedSpeedHigh,
    const char* flightPhase
);
void* receiveViolationsFromSim(void* arg);