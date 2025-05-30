#include <iostream>
#include <SFML/Graphics.hpp>
#include "ATC.hpp"
using namespace sf;

int main(){
	// ATC atc;
	// atc.initializeSystem();
	// atc.simulateFlights();

	ATC atc;
    atc.initializeSystem();

    cout << "\n=== AirControlX - Automated Air Traffic Control System ===" << endl;
    cout << "Simulation will run for 5 minutes with continuous flight scheduling." << endl;
    cout << "  - North International Arrivals: Every 3 minutes (10% emergency)" << endl;
    cout << "  - South Domestic Arrivals: Every 2 minutes (5% emergency)" << endl;
    cout << "  - East International Departures: Every 2.5 minutes (15% emergency)" << endl;
    cout << "  - West Domestic Departures: Every 4 minutes (20% emergency)" << endl;
    cout << "===========================================================" << endl;

    // Add some initial flights for demonstration
    // Current time as string

    // Start the simulation
    //atc.simulateFlights();

    atc.run();

	return 0;
}