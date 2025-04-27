#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <thread>
using namespace std;

// Mutexes for synchronization of flights
pthread_mutex_t mutexA;
pthread_mutex_t mutexB;
pthread_mutex_t mutexC;
pthread_mutex_t print_mutex;

// Colors
string black = "\033[30m";
string red = "\033[1;31m";
string green = "\033[1;32m";
string yellow = "\033[33m";
string blue = "\033[34m";
string magenta = "\033[35m";
string cyan = "\033[36m";
string white = "\033[37m";
string default_text = "\033[39m"; // added default


class Aircraft {
	string aircraftID;
	string model;
	int capacity;

public:
	Aircraft() : aircraftID(""), model(""), capacity(0) {}
	Aircraft(string id, string model, int capacity)
		: aircraftID(id), model(model), capacity(capacity) {}

	void print() const {
		cout << "Aircraft ID: " << aircraftID << ", Model: " << model
			<< ", Capacity: " << capacity << endl;
	}

	string getID() const { return aircraftID; }
};

class Runway {
	string runwayID;
	bool occupationstatus;

public:
	Runway() : runwayID(""), occupationstatus(false) {}
	Runway(string runwayID, bool occupationstatus)
		: runwayID(runwayID), occupationstatus(occupationstatus) {}

	void print() const {
		cout << "Runway ID: " << runwayID << endl;
		cout << "Occupation Status: " << (occupationstatus ? "Occupied" : "Available") << endl;
	}

	void setOccupied(bool status) { occupationstatus = status; }
	bool isOccupied() const { return occupationstatus; }

	string getRunwayID() const { return runwayID; }

	// Runway violation check based on Flight details
	bool checkRunwayViolation(const string& direction, const string& flightID) {
		if (runwayID == "RWY-A") {
			if (direction != "North" && direction != "South") {
				cout << "? Runway " << runwayID << " is incorrectly assigned for direction: " << direction << " in flight " << flightID << endl;
				return true;
			}
		}
		else if (runwayID == "RWY-B") {
			if (direction != "East" && direction != "West") {
				cout << "? Runway " << runwayID << " is incorrectly assigned for direction: " << direction << " in flight " << flightID << endl;
				return true;
			}
		}
		return false;
	}

	void setRunwayOccupied() {
		setOccupied(true);
	}

	void releaseRunway() {
		setOccupied(false);
	}
};

class Flight {
	// New data members
	string priority;
	string scheduledTime;
	


	string flightID;
	string flightType;
	string direction;
	int speed;
	string currentPhase;
	bool AVNStatus;
	Runway* assignedRunwayPtr;
	Aircraft aircraft;

	

public:
	Flight() : speed(0), AVNStatus(false), assignedRunwayPtr(nullptr) {}

	Flight(string flightID, string flightType, string direction, int speed,
		string currentPhase, Runway* runwayPtr, Aircraft aircraft)
		: flightID(flightID), flightType(flightType), direction(direction),
		speed(speed), currentPhase(currentPhase), assignedRunwayPtr(runwayPtr),
		AVNStatus(false), aircraft(aircraft) {}

	void print() const {
		cout << "\033[1;36m" << "--- Flight " << flightID << " Status ---" << "\033[0m" << endl;
		cout << "Flight ID: " << flightID << endl;
		cout << "Flight Type: " << flightType << endl;
		cout << "Direction: " << direction << endl;
		cout << "Speed: " << speed << " knots" << endl;
		cout << "Current Phase: " << currentPhase << endl;
		// cout << "Operation Type: " << (isArrival ? "Arrival" : "Departure") << endl;
		cout << "AVN Status: " << (AVNStatus ? "\033[1;31mActive\033[0m" : "\033[1;32mInactive\033[0m") << endl;
		cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
		cout << "--- Aircraft Details ---\n";
		aircraft.print();
	}

	void checkviolation() {
		if (assignedRunwayPtr->getRunwayID() == "RWY-A") {
            if (currentPhase == "Holding" && speed > 600) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Holding Phase Violation: Speed " << speed << " km/h exceeds limit of 600 km/h.  Hold 1KM around airport." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Approach" && (speed > 290 || speed < 240)) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Approach Phase Violation: Speed " << speed << " km/h is outside the 240-290 km/h range." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Landing" && (speed > 240 || speed < 30)) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Landing Phase Violation: Speed " << speed << " km/h is outside the 30-240 km/h range." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Taxi" && speed > 30) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Taxi Phase Violation: Speed " << speed << " km/h exceeds limit of 30 km/h." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "At Gate" && speed > 10) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " At Gate Phase Violation: Speed " << speed << " km/h exceeds limit of 10 km/h." << endl;
                AVNStatus = true;
            }
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-B") {
            if (currentPhase == "At Gate" && speed > 10) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " At Gate Phase Violation: Speed " << speed << " km/h exceeds limit of 10 km/h." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Taxi" && speed > 30) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Taxi Phase Violation: Speed " << speed << " km/h exceeds limit of 30 km/h." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Takeoff Roll" && (speed > 290)) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Takeoff Roll Phase Violation: Speed " << speed << " km/h exceeds limit of 290 km/h." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Climb" && (speed > 463)) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Climb Phase Violation: Speed " << speed << " km/h exceeds limit of 463 km/h." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Cruise" && (speed > 900 || speed < 800)) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Cruise Phase Violation: Speed " << speed << " km/h is outside the 800-900 km/h range." << endl;
                AVNStatus = true;
            }
        }
	}

	bool getAVNStatus() const { return AVNStatus; }
	string getID() const { return flightID; }
	string getDirection() const { return direction; }
	string getFlightType() const { return flightType; }
    string getPriority() const { return priority; }
    void setPriority(const string& p) { priority = p; }
	Runway* getAssignedRunwayPtr() const { return assignedRunwayPtr; }
	string getCurrentPhase() const { return currentPhase; }
    void setCurrentPhase(const string& phase) { currentPhase = phase; }
    int getSpeed() const { return speed; }
    void setSpeed(int s) { speed = s; }
	string getScheduledTime() const{return scheduledTime;}
    void setScheduledTime(const string& time){scheduledTime = time;}
};

class Airline {
	string name;
	string type;
	int numAircrafts;
	int numFlights;
	vector<Flight*> flights;

public:
	Airline() : numAircrafts(0), numFlights(0) {}

	Airline(string name, string type, int numAircrafts, int numFlights)
		: name(name), type(type), numAircrafts(numAircrafts), numFlights(numFlights) {}

	void addFlight(Flight* flight) {
		flights.push_back(flight);
		numFlights = flights.size();
	}

	void print() const {
		cout << "Airline: " << name << endl;
		cout << "Type: " << type << endl;
		cout << "Number of aircrafts: " << numAircrafts << endl;
		cout << "Number of flights in operation: " << numFlights << endl;

		for (const auto& f : flights) {
			cout << "\n--- Flight Details ---\n";
			f->print();
		}
	}
	vector<Flight*> getFlights() {
		return flights;
	}
	string getName() const{
		return name;
	}
};

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

	// Check which mutex to lock based on runway id
	if(runway->getRunwayID() == "RWY-A"){
		mutex = &mutexA;
	}
	else if(runway->getRunwayID() == "RWY-B"){
		mutex = &mutexB;
	}
	else if(runway->getRunwayID() == "RWY-C"){
		mutex = &mutexC;
	}
	cout << "[ " << f->getID() << " ] Requesting runway " << runway->getRunwayID() << "\n";
	// sleep(1);
	pthread_mutex_lock(mutex); // arrived flight locks the runway and other flights wait until the first one unlocks

	if(!runway->isOccupied()){
		
		cout << "[ " << f->getID() << " ] Runway assigned.\n";
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

		// unlock runway 
		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-A IS FREE------" << default_text << endl;

		sleep(2);
		randspeed = rand() % 200;
		simulatePhase("Landing", f, randspeed, mutex);
		

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

		// unlock runway 
		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-B IS FREE------" << default_text << endl;

		sleep(2);
		randspeed = rand() % 290;
		simulatePhase("Takeoff Roll", f, randspeed, mutex);

		

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

		pthread_mutex_unlock(mutex);
		cout << green << "------RUNWAY-C IS FREE------" << default_text << endl;

		sleep(2);
		randspeed = rand() % 290;
		simulatePhase("Takeoff Roll", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 250 + 200;
		simulatePhase("Climb", f, randspeed, mutex);

		sleep(1);
		randspeed = rand() % 300 + 600;
		simulatePhase("Departure", f, randspeed, mutex);
	}

	pthread_mutex_lock(mutex);
	runway->setOccupied(false);

	if(runway->getRunwayID() == "RWY-A"){
		cout << green << "------RUNWAY-A IS FREE------" << default_text << endl;
	}
	else if(runway->getRunwayID() == "RWY-B"){
		cout << green << "------RUNWAY-B IS FREE------" << default_text << endl;
	}
	else if(runway->getRunwayID() == "RWY-C"){
		cout << green << "------RUNWAY-C IS FREE------" << default_text << endl;
	}

	pthread_mutex_unlock(mutex);
	pthread_exit(NULL);
}

class AirTrafficControl {
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
	AirTrafficControl()
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

int main() {
	srand(time(0));
	AirTrafficControl atc;
	atc.initializeSystem();
	
	// atc.addFlight("PIA", "Passenger", "North", "12:00");
	// atc.addFlight("FedEx", "Cargo", "West", "13:00");
	// // atc.addFlight("AirBlue", "Passenger", "South", "14:00");
	// atc.addFlight("PIA", "Passenger", "East", "15:00");
	
	atc.simulateFlights();
	return 0;
}