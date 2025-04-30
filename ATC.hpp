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

// Mutexes for synchronization of flights
pthread_mutex_t mutexA;
pthread_mutex_t mutexB;
pthread_mutex_t mutexC;
pthread_mutex_t print_mutex;

void simulatePhase(string phase, Flight* f, int randomspeed, pthread_mutex_t* mutex){	
	f->setCurrentPhase(phase);
	f->setSpeed(randomspeed); // Example speed
	f->checkviolation();

	pthread_mutex_lock(&print_mutex);
	cout << "\033[1;33m" << f->getID() << " entering " << f->getCurrentPhase() << " phase at " << f->getSpeed() << " km/h \033[0m" << endl;
	f->print();
	cout << endl;
	pthread_mutex_unlock(&print_mutex);
}

void* handleFlight(void* arg){
	Flight* f = (Flight*) arg;
	Runway* runway = f->getAssignedRunwayPtr();

	pthread_mutex_t* mutex = nullptr;

	if(runway->getRunwayID() == "RWY-A"){
		mutex = &mutexA;
	}
	else if(runway->getRunwayID() == "RWY-B"){
		mutex = &mutexB;
	}
	else if(runway->getRunwayID() == "RWY-C"){
		mutex = &mutexC;
	}

	cout << yellow << "[ " << f->getID() << " ] Requesting runway " << runway->getRunwayID() << default_text << "\n";
    
	if(!runway->isOccupied()){
		cout << green << "[ " << f->getID() << " ] Runway assigned.\n" << default_text;
		runway->setOccupied(true);
	}

	int randspeed;
	if (f->getDirection() == "North" || f->getDirection() == "South") {
		sleep(1);
		randspeed = rand() % 500 + 100;
		simulatePhase("Holding", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 260 + 40;
		simulatePhase("Approach", f, randspeed, mutex);

		pthread_mutex_lock(mutex);
		sleep(2);
		randspeed = rand() % 200;
		simulatePhase("Landing", f, randspeed, mutex);

		// unlock runway 
		runway->setOccupied(false);
		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-A IS FREE------" << default_text << endl;
		
		sleep(1);
		randspeed = rand() % 20;
		simulatePhase("Taxi", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 2;
		simulatePhase("At Gate", f, randspeed, mutex);
    }
    else if (f->getDirection() == "East" || f->getDirection() == "West"){
		sleep(1);
		randspeed = 0;
		simulatePhase("At Gate", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 20;
		simulatePhase("Taxi", f, randspeed, mutex);

		pthread_mutex_lock(mutex);
		sleep(2);
		randspeed = rand() % 290;
		simulatePhase("Takeoff Roll", f, randspeed, mutex);

		// unlock runway 
		runway->setOccupied(false);
		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-B IS FREE------" << default_text << endl;

		sleep(1);
		randspeed = rand() % 250 + 200;
		simulatePhase("Climb", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 300 + 600;
		simulatePhase("Departure", f, randspeed, mutex);
    }
	else{ //  Runway C
		sleep(1);
		randspeed = 0;
		simulatePhase("At Gate", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 20;
		simulatePhase("Taxi", f, randspeed, mutex);

		pthread_mutex_lock(mutex);
		sleep(2);
		randspeed = rand() % 290;
		simulatePhase("Takeoff Roll", f, randspeed, mutex);

		runway->setOccupied(false);
		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-C IS FREE------" << default_text << endl;

		sleep(1);
		randspeed = rand() % 250 + 200;
		simulatePhase("Climb", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 300 + 600;
		simulatePhase("Departure", f, randspeed, mutex);
	}

	// pthread_mutex_lock(mutex);
	// runway->setOccupied(false);

	// // if(runway->getRunwayID() == "RWY-A"){
	// // 	cout << green << "------RUNWAY-A IS FREE------" << default_text << endl;
	// // }
	// // else if(runway->getRunwayID() == "RWY-B"){
	// // 	cout << green << "------RUNWAY-B IS FREE------" << default_text << endl;
	// // }
	// // else if(runway->getRunwayID() == "RWY-C"){
	// // 	cout << green << "------RUNWAY-C IS FREE------" << default_text << endl;
	// // }

	// pthread_mutex_unlock(mutex);
	pthread_exit(NULL);
}

class ATC {
private:
	Runway rwyA;
	Runway rwyB;
	Runway rwyC;
	vector<Airline> airlines;
	queue<Flight*> arrivalQueue;
	queue<Flight*> departureQueue;
	vector<pthread_t> flightThreads;
	vector<Flight*> allFlights;
public:
	ATC()
	: rwyA("RWY-A", false), rwyB("RWY-B", false), rwyC("RWY-C", false) {
		pthread_mutex_init(&mutexA, NULL);
		pthread_mutex_init(&mutexB, NULL);
		pthread_mutex_init(&mutexC, NULL);
		pthread_mutex_init(&print_mutex, NULL);
	}
	void initializeSystem(){
		// Initialize Airlines
		Airline pia("PIA", "Commercial", rand() % 4 + 1, 0);
		Airline fedex("FedEx", "Cargo", rand() % 3 + 1, 0);
		// Airline emirates("Emirates", "Commercial", 2, 0);
		
		// PIA aircrafts
		vector<Aircraft> aircraftsPia = {
			{"AC-001", "Boeing 777", 300},
			{"AC-002", "Airbus A320", 180}
		};
	
		// Fedex aircrafts
		vector<Aircraft> aircraftsFedex = {
			{"AC-101", "Boeing 747", 400},
			{"AC-102", "Cessna 208", 12}
		};
	
		// PIA Flights
		vector<Flight*> piaFlights = {
			new Flight("PK-123", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[0]),
			new Flight("PK-713", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[1]),
			new Flight("PK-234", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[0]),
			new Flight("PK-456", "Passenger", "East", 0, "At Gate", &rwyB, aircraftsPia[1])
		};
	
		for (Flight* f : piaFlights) {
			pia.addFlight(f);
	
			if (f->getDirection() == "North" || f->getDirection() == "South") {
				arrivalQueue.push(f);
			}
			else if (f->getDirection() == "East" || f->getDirection() == "West") {
				departureQueue.push(f);
			}
		}
	
		// Fedex Flights
		vector<Flight*> fedexFlights = {
			new Flight("FX-789", "Cargo", "South", 400, "Holding", &rwyC, aircraftsFedex[0]),
			new Flight("FX-321", "Cargo", "West", 0, "At Gate", &rwyC, aircraftsFedex[1])
		};
	
		for (Flight* f : fedexFlights) {
			fedex.addFlight(f);
	
			if (f->getDirection() == "North" || f->getDirection() == "South") {
				arrivalQueue.push(f);
			}
			else if (f->getDirection() == "East" || f->getDirection() == "West") {
				departureQueue.push(f);
			}
		}
	
		airlines.push_back(pia);
		airlines.push_back(fedex);
	}

	void addFlight(const string& airlineName, const string& flightType, const string& direction, const string& scheduledTime) {
        // Find the airline.  Assume PIA if not found.
        Airline* airlinePtr = nullptr;
        for (auto& airline : airlines) {
            if (airline.getName() == airlineName) {
                airlinePtr = &airline;
                break;
            }
        }
	
        Aircraft newAircraft("AC-999", "Generic Aircraft", 200);

        // Determine the runway.
        Runway* runwayPtr = nullptr;
		
        if (direction == "North" || direction == "South") {
            runwayPtr = &rwyA;
        } else if (direction == "East" || direction == "West") {
            runwayPtr = &rwyB;
        } else {
            runwayPtr = &rwyC; // Default to RWY-C
        }

        // Generate a unique flight ID.
        string newFlightId = airlinePtr->getName().substr(0, 2) + "-" + to_string(rand() % 10000);

        // Create the new flight.
        Flight* newFlight = new Flight(newFlightId, flightType, direction, 0, "At Gate", runwayPtr, newAircraft);
        newFlight->setScheduledTime(scheduledTime);
        if (newFlight->getDirection() == "North" || newFlight->getDirection() == "South") {
                arrivalQueue.push(newFlight);
            }
            else if (newFlight->getDirection() == "East" || newFlight->getDirection() == "West") {
                departureQueue.push(newFlight);
            }
        airlinePtr->addFlight(newFlight);
        allFlights.push_back(newFlight);
    }

    Flight* getFlightById(const string& id) {
        for (Flight* flight : allFlights) {
            if (flight->getID() == id) {
                return flight;
            }
        }
        return nullptr;
    }

    Runway* getRunwayById(const string& id){
        if(id == "RWY-A"){
            return &rwyA;
        }
        else if (id == "RWY-B"){
            return &rwyB;
        }
        else if (id == "RWY-C"){
            return &rwyC;
        }
        else{
            return nullptr;
        }
    }
    queue<Flight*> getArrivalQueue() {
        return arrivalQueue;
    }

    queue<Flight*> getDepartureQueue() {
        return departureQueue;
    }
	
	void simulateFlights() {
		cout << "=== FLIGHT SIMULATION WITH THREADS AND MUTEXES ===" << endl;
		// cout << "--- Airline Info ---\n";
		// for (const Airline& a : airlines) {
		// 	a.print();
		// 	cout << "\n";
		// }

		// // Print runway info
		// rwyA.print();
		// rwyB.print();
		// rwyC.print();
		// sleep(3);
		for (Airline& airline : airlines) {
			for (Flight* f : airline.getFlights()) {
				pthread_t tid;
				pthread_create(&tid, NULL, handleFlight, (void*)f);
				flightThreads.push_back(tid);
			}
		}
	
		// After launching all threads:
		for (auto& tid : flightThreads) {
			pthread_join(tid, NULL); // Wait for each flight to complete
		}
	}
};