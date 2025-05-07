#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include "Airline.hpp"
#include <pthread.h>
#include "Colors.hpp"
#include "ATC_to_AVN.hpp"
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

void simulatePhase(string phase, Flight* f, int randomspeed, pthread_mutex_t* mutex) 
{
    f->setCurrentPhase(phase);
    f->setSpeed(randomspeed);
    
    pthread_mutex_lock(mutex);
    cout << "\033[1;33m" << f->getID() << " entering " << phase << " phase at " << randomspeed << " km/h \033[0m" << endl;
    f->print();
    cout << endl;
    pthread_mutex_unlock(mutex);
    
    // Check for speed violations
    bool isViolation = false;
    int allowedSpeedHigh = 0;
    int allowedSpeedLow = 0;
    
    // This code mirrors the violation detection logic from ATCSController::detectViolations
    if(f->getAssignedRunwayPtr()->getRunwayID() == "RWY-A") {
        if(phase == "Holding") {
            allowedSpeedHigh = 600;
            allowedSpeedLow = 290;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Approach") {
            allowedSpeedHigh = 290;
            allowedSpeedLow = 240;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Landing") {
            allowedSpeedHigh = 240;
            allowedSpeedLow = 30;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Taxi") {
            allowedSpeedHigh = 30;
            allowedSpeedLow = 10;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "At Gate") {
            allowedSpeedHigh = 10;
            allowedSpeedLow = 0;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
    }
    else if(f->getAssignedRunwayPtr()->getRunwayID() == "RWY-B") {
        if(phase == "At Gate") {
            allowedSpeedHigh = 10;
            allowedSpeedLow = 0;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Taxi") {
            allowedSpeedHigh = 30;
            allowedSpeedLow = 10;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Takeoff Roll") {
            allowedSpeedHigh = 290;
            allowedSpeedLow = 30;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Climb") {
            allowedSpeedHigh = 463;
            allowedSpeedLow = 290;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
        else if(phase == "Cruise") {
            allowedSpeedHigh = 900;
            allowedSpeedLow = 800;
            if(randomspeed > allowedSpeedHigh || randomspeed < allowedSpeedLow) {
                isViolation = true;
            }
        }
    }
    
    // If violation detected, send it to AVN process via pipe
    if (isViolation) {
        cout << red << "VIOLATION DETECTED in simulation: " << f->getID()
            << " - " << phase << " phase at " << randomspeed << " km/h (limit: " << allowedSpeedHigh << " - "
            << allowedSpeedLow << " km/h)" << default_text << endl;
            
        // Get airline name
        string airlineName = "Unknown";
        if (f->getParentAirline() != NULL) {
            airlineName = f->getParentAirline()->getName();
        }

		pthread_mutex_t violation_mutex;
        pthread_mutex_init(&violation_mutex, NULL);
        pthread_mutex_lock(&violation_mutex);
        
        // Use a separate thread to send violation to avoid blocking
        pthread_t violation_thread;
        struct ViolationData {
            string id;
            string airline;
            string flightType;
            int speed;
            int minSpeed;
            int maxSpeed;
            string phase;
        };
        
        ViolationData* data = new ViolationData{
            f->getID(),
            airlineName,
            f->getFlightType(),
            randomspeed,
            allowedSpeedLow,
            allowedSpeedHigh,
            phase
        };
        
        pthread_create(&violation_thread, NULL, [](void* arg) -> void* {
            ViolationData* data = static_cast<ViolationData*>(arg);
            
            // Try to send violation with a timeout
            bool success = false;
            int retries = 3;
            
            while (retries > 0 && !success) {
                try {
                    // Add timeout or non-blocking mode
                    success = sendViolationToAVNProcess(
                        data->id.c_str(),
                        data->airline.c_str(),
                        data->flightType.c_str(),
                        data->speed,
                        data->minSpeed,
                        data->maxSpeed,
                        data->phase.c_str()
                    );
                } catch (...) {
                    // Handle any exceptions
                    cout << red << "Error sending violation to AVN process, retrying..." << default_text << endl;
                }
                
                if (!success) {
                    retries--;
                    // Short sleep before retry
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            
            if (!success) {
                cout << red << "Failed to send violation to AVN process after retries!" << default_text << endl;
            }
            
            delete data;
            return nullptr;
        }, data);
        
        // Detach thread to avoid having to join it later
        pthread_detach(violation_thread);
        pthread_mutex_unlock(&violation_mutex);
        
        // Mark flight as having an AVN
        f->setAVNStatus(true);
    }
    
}

void* handleFlight(void* arg) 
{
	Flight* f = (Flight*)arg;
	Runway* runway = f->getAssignedRunwayPtr();

	pthread_mutex_t* mutex = nullptr;

	// Check which mutex to lock based on runway id
	if (runway->getRunwayID() == "RWY-A") {
		mutex = &mutexA;
	}
	else if (runway->getRunwayID() == "RWY-B") {
		mutex = &mutexB;
	}
	else if (runway->getRunwayID() == "RWY-C") {
		mutex = &mutexC;
	}

	// pthread_mutex_lock(mutex);
	

    cout << yellow << "[ " << f->getID() << " ] Requesting runway " << runway->getRunwayID() << default_text << "\n";

    switch (f->getPriority())
    {
    case 0:
        cout << red << "Emergency Priority!" << default_text;
        break;
    case 1:
        cout << yellow << "VIP Priority" << default_text;
        break;
    case 2:
        cout << cyan << "Cargo Flight" << default_text;
        break;
    default:
        break;
    }
    cout << endl;

	// sleep(1);
	// pthread_mutex_lock(mutex); // arrived flight locks the runway and other flights wait until the first one unlocks

	if(!runway->isOccupied()){
		cout << green << "[ " << f->getID() << " ] Runway assigned.\n" << default_text;
		runway->setOccupied(true);
	}


	// sleep(1);

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

        //if (!checkGroundFaults(f)) 
        //{
            sleep(1);
            randspeed = rand() % 2;
            simulatePhase("At Gate", f, randspeed, mutex);

            // Check for ground faults at gate
            //checkGroundFaults(f);
        //}
	}
	else if (f->getDirection() == "East" || f->getDirection() == "West") {
		sleep(1);
		randspeed = 0;
		simulatePhase("At Gate", f, randspeed, mutex);
        
        //if (!checkGroundFaults(f))
        //{
            sleep(1);
            randspeed = rand() % 20;
            simulatePhase("Taxi", f, randspeed, mutex);
        //}

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
	else { //  Runway C
		sleep(1);
		randspeed = 0;
		simulatePhase("At Gate", f, randspeed, mutex);

        //if (!checkGroundFaults(f))
        //{
            sleep(1);
            randspeed = rand() % 20;
            simulatePhase("Taxi", f, randspeed, mutex);
        //}

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

	if(runway->getRunwayID() == "RWY-A"){
		cout << green << "------RUNWAY-A IS FREE------" << default_text << endl;
	}
	else if(runway->getRunwayID() == "RWY-B"){
		cout << green << "------RUNWAY-B IS FREE------" << default_text << endl;
	}
	else if(runway->getRunwayID() == "RWY-C"){
		cout << green << "------RUNWAY-C IS FREE------" << default_text << endl;
	}

	// pthread_mutex_unlock(mutex);
	pthread_exit(NULL);

}

struct FlightComparator
{
	bool operator()(Flight* a, Flight* b) const
	{
		//Higher Priority will come first
		if(a->getPriority() != b->getPriority())
		{
			return a->getPriority() < b->getPriority();
		}

		//If they have the same priortity, FCFS
		return a->getScheduledTime() < b->getScheduledTime();
	}
};

class ATC {
	private:
		std::chrono::time_point<std::chrono::steady_clock> simulationStartTime;
		const int SIMULATION_DURATION_SECONDS = 300; // 5 minutes
		bool simulationEnded = false;
		Runway rwyA;
		Runway rwyB;
		Runway rwyC;
		vector<Airline> airlines;
	
		//queue<Flight*> arrivalQueue;
		//queue<Flight*> departureQueue;
	
		// Replaced these two with the above
		struct FlightComparator
		{
			bool operator()(Flight* a, Flight* b) const
			{
				//Higher Priority will come first
				if(a->getPriority() != b->getPriority())
				{
					return a->getPriority() > b->getPriority();
				}
	
				//If they have the same priortity, FCFS
				return a->getScheduledTime() > b->getScheduledTime();
			}
		};
	
		priority_queue<Flight*, vector<Flight*>, FlightComparator> arrivalQueue;
		priority_queue<Flight*, vector<Flight*>, FlightComparator> departureQueue;
	
		sf::RenderWindow window;
		sf::Sprite sprite;
		sf::Texture texture;
		
		vector<pthread_t> flightThreads;
		vector<Flight*> allFlights;
	
		//added this
		unordered_map<string, int> estimatedWaitTimes;
	public:
		ATC()
		: rwyA("RWY-A", false), rwyB("RWY-B", false), rwyC("RWY-C", false), window(VideoMode(1500, 820), "ATC Simulator") {
			texture.loadFromFile("atc.png");
			sprite.setTexture(texture);
			pthread_mutex_init(&mutexA, NULL);
			pthread_mutex_init(&mutexB, NULL);
			pthread_mutex_init(&mutexC, NULL);
			pthread_mutex_init(&print_mutex, NULL);
		}
	
		void run() {
			window.setFramerateLimit(60);
		
			while (window.isOpen()) {
				Event event;
				while (window.pollEvent(event)) {
					if (event.type == Event::Closed)
						window.close();
				}
		
				window.clear();
				window.draw(sprite);
				
				// Add safety checks and logging
				if (!allFlights.empty()) {
					if (allFlights[0] != nullptr) {
						try {
							window.draw(allFlights[0]->getSprite());
						} catch (const std::exception& e) {
							cout << "Exception when drawing sprite: " << e.what() << endl;
						}
					} else {
						cout << "Flight pointer is null!" << endl;
					}
				} else {
					cout << "allFlights vector is empty!" << endl;
				}
				
				window.display();
			}
		}
	
		//added this
		void updateEstimatedWaitTimes() {
			// Make a copy of the queues to avoid modifying them
			auto arrivalQueueCopy = arrivalQueue;
			auto departureQueueCopy = departureQueue;
	
			int arrivalWaitTime = 0;
			int departureWaitTime = 0;
	
			// Process arrivals
			// while (!arrivalQueueCopy.empty()) {
			//     Flight* flight = arrivalQueueCopy.top();
			//     arrivalQueueCopy.pop();
	
			//     // Store the estimated wait time for this flight
			//     estimatedWaitTimes[flight->getID()] = arrivalWaitTime;
	
			//     // Each flight takes approximately 1 minute to process
			//     arrivalWaitTime += 60;
			// }
	
			// // Process departures
			// while (!departureQueueCopy.empty()) {
			//     Flight* flight = departureQueueCopy.top();
			//     departureQueueCopy.pop();
	
			//     // Store the estimated wait time for this flight
			//     estimatedWaitTimes[flight->getID()] = departureWaitTime;
	
			//     // Each flight takes approximately 1 minute to process
			//     departureWaitTime += 60;
			// }
	
			// Process arrivals - more sophisticated model
			while (!arrivalQueueCopy.empty()) 
			{
				Flight* flight = arrivalQueueCopy.top();
				arrivalQueueCopy.pop();
		
				// Base processing time based on flight type
				int processingTime = 60; // Default 1 minute
		
				// Adjust for flight type
				if (flight->getFlightType() == "Cargo") 
				{
				processingTime += 20; // Cargo takes longer
				} 
				else if (flight->getFlightType() == "Medical") 
				{
				processingTime -= 20; // Medical gets expedited (minimum 40 seconds)
				}
		
				// Adjust for runway congestion
				if (rwyA.isOccupied() && (flight->getDirection() == "North" || flight->getDirection() == "South")) 
				{
				processingTime += 30; // Add delay if preferred runway is occupied
				}
		
				// Store the estimated wait time for this flight
				estimatedWaitTimes[flight->getID()] = arrivalWaitTime;
		
				// Each flight adds to the cumulative wait time
				arrivalWaitTime += processingTime;
			}
	
			while (!departureQueueCopy.empty()) 
			{
				Flight* flight = departureQueueCopy.top();
				departureQueueCopy.pop();
		
				int processingTime = 50; // Default departure time
		
				if (flight->getFlightType() == "Cargo") 
				{
					processingTime += 30; // Cargo loading takes longer
				} 
				else if (flight->getFlightType() == "Military") 
				{
					processingTime -= 15; // Military gets expedited
				}
		
				// Adjust for runway congestion
				if (rwyB.isOccupied() && (flight->getDirection() == "East" || flight->getDirection() == "West")) 
				{
					processingTime += 40; // Add delay if preferred runway is occupied
				}
		
				// Store the estimated wait time for this flight
				estimatedWaitTimes[flight->getID()] = departureWaitTime;
		
				// Each flight adds to the cumulative wait time
				departureWaitTime += processingTime;
			}
	
		}
	
		std::string getFormattedWaitTime(const std::string& flightId) 
		{
			int seconds = getEstimatedWaitTime(flightId);
			if (seconds < 0) {
				return "Unknown";
			}
		
			int minutes = seconds / 60;
			seconds %= 60;
		
			return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
		}
	
		int getEstimatedWaitTime(const string& flightId) {
			if (estimatedWaitTimes.find(flightId) != estimatedWaitTimes.end()) {
				return estimatedWaitTimes[flightId];
			}
			return -1; // Not found
		}
	
		//added this
		void displayFlightStatus() 
		{
			pthread_mutex_lock(&print_mutex);
			cout << "\n" << cyan << "======== CURRENT FLIGHT STATUS ========" << default_text << endl;
			cout << "Total Flights: " << allFlights.size() << endl;
	
			// Track flights by phase and type
			int holding = 0, approach = 0, landing = 0, taxi = 0, atGate = 0;
			int takeoff = 0, climb = 0, departure = 0;
			int activeViolations = 0;
			int passengerFlights = 0, cargoFlights = 0, militaryFlights = 0, medicalFlights = 0;
			int emergencyPriority = 0, vipPriority = 0, normalPriority = 0;
	
			for (Flight* flight : allFlights) 
			{
				// Count by phase
				string phase = flight->getCurrentPhase();
				if (phase == "Holding") holding++;
				else if (phase == "Approach") approach++;
				else if (phase == "Landing") landing++;
				else if (phase == "Taxi") taxi++;
				else if (phase == "At Gate") atGate++;
				else if (phase == "Takeoff Roll") takeoff++;
				else if (phase == "Climb") climb++;
				else if (phase == "Departure") departure++;
	
				// Count by type
				string type = flight->getFlightType();
				if (type == "Passenger") passengerFlights++;
				else if (type == "Cargo") cargoFlights++;
				else if (type == "Military") militaryFlights++;
				else if (type == "Medical") medicalFlights++;
	
				// Count by priority
				int priority = flight->getPriority();
				if (priority == 0) emergencyPriority++;
				else if (priority == 1) vipPriority++;
				else normalPriority++;
	
				// Count violations
				if (flight->getAVNStatus()) 
				{
					activeViolations++;
				}
			}
	
			// Display flight breakdown by phase
			cout << "\nFlights by Phase:" << endl;
			cout << " " << blue << "Holding: " << holding << default_text << " | "
			<< magenta << "Approach: " << approach << default_text << " | "
			<< yellow << "Landing: " << landing << default_text << " | "
			<< cyan << "Taxi: " << taxi << default_text << endl;
			cout << " " << green << "At Gate: " << atGate << default_text << " | "
			<< red << "Takeoff: " << takeoff << default_text << " | "
			<< blue << "Climb: " << climb << default_text << " | "
			<< magenta << "Departure: " << departure << default_text << endl;
	
			// Display flight breakdown by type
			cout << "\nFlights by Type:" << endl;
			cout << " " << white << "Passenger: " << passengerFlights << default_text << " | "
			<< cyan << "Cargo: " << cargoFlights << default_text << " | "
			<< yellow << "Military: " << vipPriority << default_text << " | "
			<< red << "Medical: " << medicalFlights << default_text << endl;
	
			// Display priority breakdown
			cout << "\nPriority Levels:" << endl;
			cout << " " << red << "Emergency: " << emergencyPriority << default_text << " | "
			<< yellow << "VIP: " << vipPriority << default_text << " | "
			<< white << "Normal: " << normalPriority << default_text << endl;
	
			// Active violations
			cout << "\nActive Violations: " << activeViolations << endl;
			if (activeViolations > 0) 
			{
				cout << "Aircraft with Violations:" << endl;
				for (Flight* flight : allFlights) 
				{
					if (flight->getAVNStatus()) 
					{
						cout << " " << red << flight->getID() << default_text << " ("
						<< flight->getFlightType() << ", " << flight->getCurrentPhase() << ")" << endl;
					}
				}
			}
	
			// Runway status
			cout << "\nRunway Status:" << endl;
			cout << " RWY-A: " << (rwyA.isOccupied() ? red + "Occupied" : green + "Available") << default_text << endl;
			cout << " RWY-B: " << (rwyB.isOccupied() ? red + "Occupied" : green + "Available") << default_text << endl;
			cout << " RWY-C: " << (rwyC.isOccupied() ? red + "Occupied" : green + "Available") << default_text << endl;
	
			// Queue lengths
			cout << "\nQueue Status:" << endl;
			auto arrivalQueueCopy = arrivalQueue;
			auto departureQueueCopy = departureQueue;
			cout << " Arrival Queue: " << arrivalQueueCopy.size() << " flights waiting" << endl;
			cout << " Departure Queue: " << departureQueueCopy.size() << " flights waiting" << endl;
	
			// Update wait times before displaying them
			updateEstimatedWaitTimes();
	
			// Show next flights in each queue with estimated wait times
			if (!arrivalQueueCopy.empty()) 
			{
				Flight* nextArrival = arrivalQueueCopy.top();
				cout << "\nNext Arrival: " << nextArrival->getID()
				<< " - Est. Wait: " << getFormattedWaitTime(nextArrival->getID()) << endl;
			}
	
			if (!departureQueueCopy.empty()) 
			{
				Flight* nextDeparture = departureQueueCopy.top();
				cout << "Next Departure: " << nextDeparture->getID()
				<< " - Est. Wait: " << getFormattedWaitTime(nextDeparture->getID()) << endl;
			}
	
			pthread_mutex_unlock(&print_mutex);
		}
	
		void initializeSystem() {
			// Initialize Airlines
			Airline pia("PIA", "Commercial", rand() % 4 + 1, 0);
			Airline fedex("FedEx", "Cargo", rand() % 3 + 1, 0);
			Airline airblue("AirBlue", "Commercial", rand() % 4 + 1, 0);
			Airline pakairforce("Pakistan AirForce", "Military", rand() % 2 + 1, 0);
			Airline bluedart("Blue Dart", "Cargo", rand() % 2 + 1, 0);
			Airline aghakhan("Agha Khan Air Ambulance", "Medical", rand() % 2 + 1, 0);
			// PIA aircrafts
			vector<Aircraft> aircraftsPia = {
				{"AC-001", "Boeing 777", 300},
				{"AC-002", "Airbus A320", 180},
				{"AC-003", "Boeing 787", 250},
				{"AC-004", "Airbus A330", 290},
				{"AC-005", "ATR 72", 70},
				{"AC-006", "Boeing 737", 150}
			};
	
			// Fedex aircrafts
			vector<Aircraft> aircraftsFedex = {
				{"AC-101", "Boeing 747", 400},
				{"AC-102", "Cessna 208", 12},
				{"AC-103", "MD-11F", 350}
			};
	
			// AirBlue aircrafts (4 required)
			vector<Aircraft> aircraftsAirBlue = {
				{"AB-101", "Airbus A320", 180},
				{"AB-102", "Airbus A321", 200},
				{"AB-103", "Boeing 737", 160},
				{"AB-104", "Airbus A319", 140}
			};
	
			vector<Aircraft> aircraftsPAF = {
				{"PAF-001", "C-130 Hercules", 92},
				{"PAF-002", "F-16 Fighting Falcon", 1}
			};
			
			vector<Aircraft> aircraftsBlueDart = {
				{"BD-201", "Boeing 757 Freighter", 0},
				{"BD-202", "ATR 72 Freighter", 0}
			};
	
			vector<Aircraft> aircraftsAGA = {
				{"AGA-301", "Bell 407 Helicopter", 6},
				{"AGA-302", "Beechcraft King Air", 8}
			};
	
			// PIA Flights
			vector<Flight*> piaFlights = {
				new Flight("PK-123", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[0]),
				new Flight("PK-713", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[1]),
				new Flight("PK-234", "Passenger", "North", 400, "Holding", &rwyA, aircraftsPia[2]),
				new Flight("PK-456", "Passenger", "East", 0, "At Gate", &rwyB, aircraftsPia[3])
			};
	
			for (Flight* f : piaFlights) {
				pia.addFlight(f);
				allFlights.push_back(f); //ok lol
	
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
				allFlights.push_back(f); //ok lol
	
				if (f->getDirection() == "North" || f->getDirection() == "South") {
					arrivalQueue.push(f);
				}
				else if (f->getDirection() == "East" || f->getDirection() == "West") {
					departureQueue.push(f);
				}
			}
	
			//AirBlue Flights
			// Assuming AirBlue has 4 flights
			vector<Flight*> airBlueFlights = {
				new Flight("AB-501", "Passenger", "North", 400, "Holding", &rwyA, aircraftsAirBlue[0]),
				new Flight("AB-502", "Passenger", "South", 400, "Holding", &rwyA, aircraftsAirBlue[1]),
				new Flight("AB-503", "Passenger", "East", 0, "At Gate", &rwyB, aircraftsAirBlue[2]),
				new Flight("AB-504", "Passenger", "West", 0, "At Gate", &rwyB, aircraftsAirBlue[3])
			};
			
			for (Flight* f : airBlueFlights) {
				airblue.addFlight(f);
				allFlights.push_back(f);
				
				if (f->getDirection() == "North" || f->getDirection() == "South") {
					arrivalQueue.push(f);
				}
				else if (f->getDirection() == "East" || f->getDirection() == "West") {
					departureQueue.push(f);
				}
			}
	
			// Pakistan Airforce Flights (1 required)
			vector<Flight*> pafFlights = {
				new Flight("PAF-101", "Military", "East", 0, "At Gate", &rwyB, aircraftsPAF[0])
			};
	
			for (Flight* f : pafFlights) {
				pakairforce.addFlight(f);
				allFlights.push_back(f);
				
				if (f->getDirection() == "East" || f->getDirection() == "West") {
					departureQueue.push(f);
				}
			}
	
			//Blue Dart Flights
			vector<Flight*> blueDartFlights = {
				new Flight("BD-301", "Cargo", "South", 400, "Holding", &rwyC, aircraftsBlueDart[0]),
				new Flight("BD-302", "Cargo", "West", 0, "At Gate", &rwyC, aircraftsBlueDart[1])
			};
			
			for (Flight* f : blueDartFlights) {
				bluedart.addFlight(f);
				allFlights.push_back(f);
				
				if (f->getDirection() == "North" || f->getDirection() == "South") {
					arrivalQueue.push(f);
				}
				else if (f->getDirection() == "East" || f->getDirection() == "West") {
					departureQueue.push(f);
				}
			}
	
			//Agha Khan Air Ambulance Flights
			vector<Flight*> agaFlights = {
				new Flight("AGA-201", "Medical", "North", 400, "Holding", &rwyA, aircraftsAGA[0])
			};
			
			for (Flight* f : agaFlights) {
				aghakhan.addFlight(f);
				allFlights.push_back(f);
				
				// Set emergency priority for medical flights
				f->setPriority(0);
				
				if (f->getDirection() == "North" || f->getDirection() == "South") {
					arrivalQueue.push(f);
				}
			}
	
			airlines.push_back(pia);
			airlines.push_back(fedex);
			airlines.push_back(airblue);
			airlines.push_back(pakairforce);
			airlines.push_back(bluedart);
			airlines.push_back(aghakhan);
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
	
			if (!airlinePtr && !airlines.empty()) 
			{
				airlinePtr = &airlines[0];
				cout << yellow << "Warning: Airline " << airlineName << " not found. Using " 
					 << airlinePtr->getName() << " instead." << default_text << endl;
			} else if (!airlinePtr) 
			{
				cout << red << "Error: No airlines available to add flight to." << default_text << endl;
				return;
			}
	
			// Generate a unique flight ID.
			string newFlightId = airlinePtr->getName().substr(0, 2) + "-" + to_string(rand() % 10000);
	
			Aircraft newAircraft("AC-" + to_string(rand() % 1000), "Generic Aircraft", 200);
	
			// Determine the runway.
			/*Runway* runwayPtr = nullptr;
	
			if (direction == "North" || direction == "South") {
				runwayPtr = &rwyA;
			}
			else if (direction == "East" || direction == "West") {
				runwayPtr = &rwyB;
			}
			else {
				runwayPtr = &rwyC; // Default to RWY-C
			}*/
	
	
	
			// Create the new flight.
			Flight* newFlight = new Flight(newFlightId, flightType, direction, 0, "At Gate", nullptr, newAircraft);
			newFlight->setScheduledTime(scheduledTime);
	
			cout << "\n\n\nI just wanna see the scheduledTime: " << scheduledTime << "\n\n\n";
	
			Runway* runwayPtr = allocateRunway(newFlight);
			if (!runwayPtr) 
			{
				cout << red << "Error: Could not allocate runway for flight " << newFlightId << default_text << endl;
				return;
			}
	
			newFlight->setAssignedRunwayPtr(runwayPtr);
	
			if (newFlight->getDirection() == "North" || newFlight->getDirection() == "South") {
				arrivalQueue.push(newFlight);
			}
			else if (newFlight->getDirection() == "East" || newFlight->getDirection() == "West") {
				departureQueue.push(newFlight);
			}
			airlinePtr->addFlight(newFlight);
			allFlights.push_back(newFlight);
	
			cout << green << "Added new flight " << newFlightId << " scheduled for " << scheduledTime << default_text << endl;
	
		}
	
		Flight* getFlightById(const string& id) {
			for (Flight* flight : allFlights) {
				if (flight->getID() == id) {
					return flight;
				}
			}
			return nullptr;
		}
	
		Runway* getRunwayById(const string& id) {
			if (id == "RWY-A") {
				return &rwyA;
			}
			else if (id == "RWY-B") {
				return &rwyB;
			}
			else if (id == "RWY-C") {
				return &rwyC;
			}
			else {
				return nullptr;
			}
		}
		priority_queue<Flight*, vector<Flight*>, FlightComparator> getArrivalQueue() {
			return arrivalQueue;
		}
	
		priority_queue<Flight*, vector<Flight*>, FlightComparator> getDepartureQueue() {
			return departureQueue;
		}
	
		void printPriorityQueue(priority_queue<Flight*, vector<Flight*>, FlightComparator> pq) {
			// We need to copy the queue since pop() modifies it
			auto queueCopy = pq;
			int position = 1;
			
			while (!queueCopy.empty()) {
				Flight* flight = queueCopy.top();
				queueCopy.pop();
				
				// Print position, ID, priority type, and scheduled time
				cout << position << ". Flight " << flight->getID() 
					 << " (";
				
				// Convert priority number to text
				switch (flight->getPriority()) {
					case 0: cout << red << "EMERGENCY" << default_text; break;
					case 1: cout << yellow << "VIP" << default_text; break;
					case 2: cout << cyan << "Cargo" << default_text; break;
					case 3: cout << white << "Commercial" << default_text; break;
					default: cout << "Unknown";
				}
				
				cout << ") - Scheduled: " << flight->getScheduledTime() 
					 << " - Direction: " << flight->getDirection() << endl;
				
				position++;
			}
			
			if (position == 1) {
				cout << "  Queue is empty" << endl;
			}
		}
	
		void testPriorityQueues() 
		{
			cout << "--- Current Arrival Queue Order ---" << endl;
			printPriorityQueue(arrivalQueue);
			
			cout << "\n--- Current Departure Queue Order ---" << endl;
			printPriorityQueue(departureQueue);
		}
	
		void simulateFlights() {
			cout << "=== FLIGHT SIMULATION WITH THREADS AND MUTEXES ===" << endl;
	
			simulationStartTime = std::chrono::steady_clock::now();
			simulationEnded = false;
	
			// Create a thread for the timer
			pthread_t timerThread;
			pthread_create(&timerThread, NULL, [](void* arg) -> void* {
				ATC* atc = static_cast<ATC*>(arg);
	
				while (true) {
					auto currentTime = std::chrono::steady_clock::now();
					auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
						currentTime - atc->simulationStartTime).count();
	
					int remainingSeconds = atc->SIMULATION_DURATION_SECONDS - elapsedSeconds;
	
					if (remainingSeconds <= 0) {
						pthread_mutex_lock(&print_mutex);
						cout << red << "\n=== SIMULATION TIME EXPIRED (5 MINUTES) ===" << default_text << endl;
						pthread_mutex_unlock(&print_mutex);
						atc->simulationEnded = true;
						break;
					}
	
					// Update status every 15 seconds
					if (elapsedSeconds % 15 == 0)
					{
						pthread_mutex_lock(&print_mutex);
						cout << yellow << "\n=== SIMULATION TIME: " << elapsedSeconds
						<< " seconds / " << remainingSeconds << " seconds remaining ==="
						<< default_text << endl;
						pthread_mutex_unlock(&print_mutex);
					
						// Display comprehensive status
						atc->displayFlightStatus();
					
						// Update wait times
						atc->updateEstimatedWaitTimes();
					}
	   
					std::this_thread::sleep_for(std::chrono::seconds(1));
					
					}
			return nullptr;
			}, this);
	
			// Generate random emergencies
			int emergencycount = rand() % 2 + 1;
			for (int i = 0; i < emergencycount; i++) {
				if (!allFlights.empty()) {
					int index = rand() % allFlights.size();
					Flight* emergencyflight = allFlights[index];
	
					if (emergencyflight->getPriority() != 0) {
						declareEmergency(emergencyflight);
					}
				}
			}
	
			cout << yellow << "\nQueues before simulation:" << default_text << endl;
			testPriorityQueues();
			cout << endl;
	
			// Create a thread for continuous flight generation
			pthread_t flightGenThread;
			pthread_create(&flightGenThread, NULL, [](void* arg) -> void* {
				ATC* atc = static_cast<ATC*>(arg);
	
				// Flight generation timers (in seconds)
				const int northArrivalInterval = 180; // Every 3 minutes
				const int southArrivalInterval = 120; // Every 2 minutes
				const int eastDepartureInterval = 150; // Every 2.5 minutes
				const int westDepartureInterval = 240; // Every 4 minutes
	
				int northTimer = 0;
				int southTimer = 0;
				int eastTimer = 0;
				int westTimer = 0;
	
				// Generate flights according to schedule until simulation ends
				while (!atc->simulationEnded) {
					// Sleep for 1 second
					std::this_thread::sleep_for(std::chrono::seconds(1));
	
					// Increment timers
					northTimer++;
					southTimer++;
					eastTimer++;
					westTimer++;
	
					// Check if it's time to generate a flight
					if (northTimer >= northArrivalInterval) {
						// Reset timer
						northTimer = 0;
	
						// 10% chance of emergency for North flights
						bool isEmergency = (rand() % 100 < 10);
						string flightType = isEmergency ? "Medical" : "Passenger";
	
						// Generate time string (current time + random offset)
						auto now = std::chrono::system_clock::now();
						auto in_time_t = std::chrono::system_clock::to_time_t(now);
						char timeStr[9]; 
						std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&in_time_t));
	
						pthread_mutex_lock(&print_mutex);
						atc->addFlight("PIA", flightType, "North", timeStr);
						if (isEmergency) {
							Flight* newFlight = atc->allFlights.back();
							atc->declareEmergency(newFlight);
						}
						pthread_mutex_unlock(&print_mutex);
					}
	
					if (southTimer >= southArrivalInterval) {
						southTimer = 0;
	
						// 5% chance of emergency for South flights
						bool isEmergency = (rand() % 100 < 5);
						string flightType = isEmergency ? "Medical" : "Passenger";
	
						auto now = std::chrono::system_clock::now();
						auto in_time_t = std::chrono::system_clock::to_time_t(now);
						char timeStr[9]; 
						std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&in_time_t));
	
						pthread_mutex_lock(&print_mutex);
						atc->addFlight("AirBlue", flightType, "South", timeStr);
						if (isEmergency) {
							Flight* newFlight = atc->allFlights.back();
							atc->declareEmergency(newFlight);
						}
						pthread_mutex_unlock(&print_mutex);
					}
	
					if (eastTimer >= eastDepartureInterval) {
						eastTimer = 0;
	
						// 15% chance of emergency for East flights
						bool isEmergency = (rand() % 100 < 15);
						string flightType = isEmergency ? "Military" : "Passenger";
	
						auto now = std::chrono::system_clock::now();
						auto in_time_t = std::chrono::system_clock::to_time_t(now);
						char timeStr[9]; 
						std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&in_time_t));
	
						pthread_mutex_lock(&print_mutex);
						atc->addFlight(isEmergency ? "Pakistan AirForce" : "PIA", flightType, "East", timeStr);
						if (isEmergency) {
							Flight* newFlight = atc->allFlights.back();
							atc->declareEmergency(newFlight);
						}
						pthread_mutex_unlock(&print_mutex);
					}
	
					if (westTimer >= westDepartureInterval) {
						westTimer = 0;
	
						// 20% chance of emergency for West flights
						bool isEmergency = (rand() % 100 < 20);
						string flightType;
						string airline;
	
						if (isEmergency) {
							// Randomly choose between VIP (military) or urgent cargo
							if (rand() % 2 == 0) {
								flightType = "Military";
								airline = "Pakistan AirForce";
							}
							else {
								flightType = "Cargo";
								airline = "FedEx";
							}
						}
						else {
							flightType = "Passenger";
							airline = "AirBlue";
						}
	
						auto now = std::chrono::system_clock::now();
						auto in_time_t = std::chrono::system_clock::to_time_t(now);
						char timeStr[9]; 
						std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&in_time_t));
	
						pthread_mutex_lock(&print_mutex);
						atc->addFlight(airline, flightType, "West", timeStr);
						if (isEmergency) {
							Flight* newFlight = atc->allFlights.back();
							atc->declareEmergency(newFlight);
						}
						pthread_mutex_unlock(&print_mutex);
					}
				}
	
				return nullptr;
				}, this);
	
			// Create a thread to process the flight queues
			pthread_t flightProcessThread;
			pthread_create(&flightProcessThread, NULL, [](void* arg) -> void* {
				ATC* atc = static_cast<ATC*>(arg);
	
				while (!atc->simulationEnded) {
					// Process arrivals
					if (!atc->arrivalQueue.empty()) {
						Flight* nextArrival = atc->arrivalQueue.top();
	
						// Check if the runway is available
						Runway* runway = nextArrival->getAssignedRunwayPtr();
						if (runway && !runway->isOccupied()) {
							atc->arrivalQueue.pop();
	
							// Launch flight thread
							pthread_t tid;
							pthread_create(&tid, NULL, handleFlight, (void*)nextArrival);
							atc->flightThreads.push_back(tid);
						}
					}
	
					// Process departures
					if (!atc->departureQueue.empty()) {
						Flight* nextDeparture = atc->departureQueue.top();
	
						// Check if the runway is available
						Runway* runway = nextDeparture->getAssignedRunwayPtr();
						if (runway && !runway->isOccupied()) {
							atc->departureQueue.pop();
	
							// Launch flight thread
							pthread_t tid;
							pthread_create(&tid, NULL, handleFlight, (void*)nextDeparture);
							atc->flightThreads.push_back(tid);
						}
					}
	
					// Sleep a bit to avoid busy waiting
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}
	
				return nullptr;
				}, this);
	
			// Wait for the simulation time to expire
			pthread_join(timerThread, NULL);
	
			// Signal other threads to terminate
			simulationEnded = true;
	
			// Wait for the flight generation and processing threads to finish
			pthread_join(flightGenThread, NULL);
			pthread_join(flightProcessThread, NULL);
	
			// Wait for all flight threads to complete
			for (auto& tid : flightThreads) {
				pthread_join(tid, NULL);
			}
	
			cout << yellow << "\n=== SIMULATION COMPLETED ===" << default_text << endl;
		}
	
	
		bool isEmergency(Flight* flight)
		{
			return flight->getPriority() == 0; //Emergency Priority
		}
	
		Runway* getPrimaryRunway(Flight* flight)
		{
			if(flight->getDirection() == "North" || flight->getDirection() == "South")
			{
				return &rwyA;
			}
			else
			{
				return &rwyB;
			}
		}
	
		Runway* allocateRunway(Flight* flight)
		{
			// Medical flights get highest priority
			if(flight->getFlightType() == "Medical")
			{
				// Try to get any available runway
				if(!rwyA.isOccupied()) return &rwyA;
				if(!rwyB.isOccupied()) return &rwyB;
				if(!rwyC.isOccupied()) return &rwyC;
				
				// If all occupied, default to direction-based runway
				return getPrimaryRunway(flight);
			}
			
			// Military flights get second priority
			if(flight->getFlightType() == "Military")
			{
				// Try to get any available runway except C (reserved for cargo)
				if(!rwyA.isOccupied()) return &rwyA;
				if(!rwyB.isOccupied()) return &rwyB;
				
				// If all occupied, default to direction-based runway
				return getPrimaryRunway(flight);
			}
			
			// Cargo flights use runway C
			if(flight->getFlightType() == "Cargo")
			{
				return &rwyC;
			}
			
			// Regular emergency handling
			if(isEmergency(flight))
			{
				Runway* primary = getPrimaryRunway(flight);
				if(!primary->isOccupied())
				{
					return primary;
				}
		
				if(!rwyC.isOccupied())
				{
					return &rwyC;
				}
			}
		
			// Standard direction-based allocation
			if(flight->getDirection() == "North" || flight->getDirection() == "South")
			{
				return &rwyA;
			}
			else
			{
				return &rwyB;
			}
		}
	
		void declareEmergency(Flight* flight) 
		{
			int oldPriority = flight->getPriority();
			flight->setPriority(0); // Set to Emergency priority
		   
			if (oldPriority != 0) 
			{
				pthread_mutex_lock(&print_mutex);
				cout << red << "[ " << flight->getID() << " ] !!! EMERGENCY DECLARED !!!" << default_text << endl;
			
				// Log the type of emergency
				string emergencyType;
				switch (rand() % 5) 
				{
				case 0: emergencyType = "Medical emergency on board"; break;
				case 1: emergencyType = "Engine malfunction"; break;
				case 2: emergencyType = "Hydraulic system failure"; break;
				case 3: emergencyType = "Fuel leak"; break;
				case 4: emergencyType = "Pressurization issue"; break;
				}
				cout << red << "[ " << flight->getID() << " ] Emergency type: " << emergencyType << default_text << endl;
			
				// Re-sort the queues to reflect the new priority
				cout << yellow << "[ " << flight->getID() << " ] Re-ordering flight queues for emergency priority" << default_text << endl;
				pthread_mutex_unlock(&print_mutex);
			
				// Re-create the arrival queue with updated priorities
				vector<Flight*> tempflights;
				while (!arrivalQueue.empty()) 
				{
					tempflights.push_back(arrivalQueue.top());
					arrivalQueue.pop();
				}
			
				for (auto f : tempflights) 
				{
					arrivalQueue.push(f);
				}
			
				// Re-create the departure queue with updated priorities
				tempflights.clear();
				while (!departureQueue.empty()) 
				{
					tempflights.push_back(departureQueue.top());
					departureQueue.pop();
				}
			
				for (auto f : tempflights) 
				{
				departureQueue.push(f);
				}
			
				// Try to allocate a runway immediately if possible
				Runway* emergencyRunway = allocateRunway(flight);
			
				pthread_mutex_lock(&print_mutex);
				if (!emergencyRunway->isOccupied()) 
				{
					cout << green << "[ " << flight->getID() << " ] Emergency flight assigned to "
					<< emergencyRunway->getRunwayID() << " - IMMEDIATE CLEARANCE" << default_text << endl;
				
					// Alert emergency services
					cout << blue << "[ SYSTEM ] Emergency services notified and on standby" << default_text << endl;
				} 
				else 
				{
					cout << yellow << "[ " << flight->getID() << " ] Waiting for "
					<< emergencyRunway->getRunwayID() << " to become available" << default_text << endl;
				
					// Calculate expected wait time
					updateEstimatedWaitTimes();
					cout << yellow << "[ " << flight->getID() << " ] Estimated wait time: "
					<< getFormattedWaitTime(flight->getID()) << default_text << endl;
				}
				pthread_mutex_unlock(&print_mutex);
			}
		}
	
		bool checkGroundFaults(Flight* flight) {
			// Only check for ground faults during taxi or at gate phases
			if (flight->getCurrentPhase() == "Taxi" || flight->getCurrentPhase() == "At Gate") {
				// 5% chance of a ground fault
				if (rand() % 100 < 5) {
					string faultType;
					switch (rand() % 3) {
					case 0: faultType = "brake failure"; break;
					case 1: faultType = "hydraulic leak"; break;
					case 2: faultType = "electrical issue"; break;
					}
	
					pthread_mutex_lock(&print_mutex);
					cout << red << "[ " << flight->getID() << " ] GROUND FAULT DETECTED: "
						<< faultType << " during " << flight->getCurrentPhase() << " phase!" << default_text << endl;
					cout << yellow << "[ " << flight->getID() << " ] Aircraft will be towed and removed from operations." << default_text << endl;
	
					// Release runway if it was occupied
					if (flight->getAssignedRunwayPtr() && flight->getAssignedRunwayPtr()->isOccupied()) {
						flight->getAssignedRunwayPtr()->setOccupied(false);
						cout << green << "------RUNWAY-" << flight->getAssignedRunwayPtr()->getRunwayID().substr(4)
							<< " IS FREE (FAULT DETECTED)------" << default_text << endl;
					}
	
					pthread_mutex_unlock(&print_mutex);
	
					// Return true to indicate a fault was detected
					return true;
				}
			}
			// Return false if no fault was detected
			return false;
		}
	};