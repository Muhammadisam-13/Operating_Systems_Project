#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <thread>
#include <unordered_map>
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
    int priority; // 0 is Emergency, 1 is VIP, 2 is Cargo, 3 is Commercial
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
	Flight() : speed(0), AVNStatus(false), assignedRunwayPtr(nullptr), priority(3) {}

	Flight(string flightID, string flightType, string direction, int speed,
		string currentPhase, Runway* runwayPtr, Aircraft aircraft)
		: flightID(flightID), flightType(flightType), direction(direction),
		speed(speed), currentPhase(currentPhase), assignedRunwayPtr(runwayPtr),
		AVNStatus(false), aircraft(aircraft)
        {
            if(flightType == "Passenger")
            {
                priority = 3;
            }
            else if(flightType == "Cargo")
            {
                priority = 2;
            }
        }

	void print() const {
		cout << "\033[1;36m" << "--- Flight " << flightID << " Status ---" << "\033[0m" << endl;
		cout << "Flight ID: " << flightID << endl;
		cout << "Flight Type: " << flightType << endl;
		cout << "Direction: " << direction << endl;
		cout << "Speed: " << speed << " knots" << endl;
		cout << "Current Phase: " << currentPhase << endl;
		// cout << "Operation Type: " << (isArrival ? "Arrival" : "Departure") << endl;
		cout << "AVN Status: " << (AVNStatus ? "\033[1;31mActive\033[0m" : "\033[1;32mInactive\033[0m") << endl;
        if (assignedRunwayPtr) 
        {
            cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
        } else 
        {
            cout << "Assigned Runway: None" << endl;
        }

		cout << "--- Aircraft Details ---\n";
		aircraft.print();
        cout << "Priority: ";
        switch(priority)
        {
            case 0:
                cout << red << "Emergency" << default_text;
                break;
            case 1:
                cout << yellow << "VIP" << default_text;
                break;
            case 2:
                cout << cyan << "Cargo" << default_text;
                break;
            case 3:
                cout << white << "Commercial" << default_text;
                break;
            default:
                cout << "Unknown";
                break;
        }
        cout << endl;

	}



	void checkviolation() {
		if (assignedRunwayPtr->getRunwayID() == "RWY-A") {
            if (currentPhase == "Holding" && speed > 600) {
                cout << red << "[ " << flightID << " ]" << default_text
                    << " Holding Phase Violation: Speed " << speed << " km/h exceeds limit of 600 km/h.  Hold 1KM around airport." << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Approach" && (speed > 290 || speed < 240)) {
                cout << red << "[ " << flightID << " ]"
                    << " Approach Phase Violation: Speed " << speed << " km/h is outside the 240-290 km/h range." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Landing" && (speed > 240 || speed < 30)) {
                cout << red << "[ " << flightID << " ]"
                    << " Landing Phase Violation: Speed " << speed << " km/h is outside the 30-240 km/h range." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Taxi" && speed > 30) {
                cout << red << "[ " << flightID << " ]"
                    << " Taxi Phase Violation: Speed " << speed << " km/h exceeds limit of 30 km/h." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "At Gate" && speed > 10) {
                cout << red << "[ " << flightID << " ]"
                    << " At Gate Phase Violation: Speed " << speed << " km/h exceeds limit of 10 km/h." << default_text << endl;
                AVNStatus = true;
            }
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-B") {
            if (currentPhase == "At Gate" && speed > 10) {
                cout << red << "[ " << flightID << " ]"
                    << " At Gate Phase Violation: Speed " << speed << " km/h exceeds limit of 10 km/h." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Taxi" && speed > 30) {
                cout << red << "[ " << flightID << " ]"
                    << " Taxi Phase Violation: Speed " << speed << " km/h exceeds limit of 30 km/h." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Takeoff Roll" && (speed > 290)) {
                cout << red << "[ " << flightID << " ]"
                    << " Takeoff Roll Phase Violation: Speed " << speed << " km/h exceeds limit of 290 km/h." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Climb" && (speed > 463)) {
                cout << red << "[ " << flightID << " ]"
                    << " Climb Phase Violation: Speed " << speed << " km/h exceeds limit of 463 km/h." << default_text << endl;
                AVNStatus = true;
            }
            else if (currentPhase == "Cruise" && (speed > 900 || speed < 800)) {
                cout << red << "[ " << flightID << " ]"
                    << " Cruise Phase Violation: Speed " << speed << " km/h is outside the 800-900 km/h range." << default_text << endl;
                AVNStatus = true;
            }
        }
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
	int getPriority() const { return priority; }
	void setPriority(const int& p) { priority = p; }
	Runway* getAssignedRunwayPtr() const { return assignedRunwayPtr; }
    void setAssignedRunwayPtr(Runway* r) {assignedRunwayPtr = r;}
	string getCurrentPhase() const { return currentPhase; }
	void setCurrentPhase(const string& phase) { currentPhase = phase; }
	int getSpeed() const { return speed; }
	void setSpeed(int s) { speed = s; }
	string getScheduledTime() const { return scheduledTime; }
	void setScheduledTime(const string& time) { scheduledTime = time; }
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
	string getName() const {
		return name;
	}
};

void simulatePhase(string phase, Flight* f, int randomspeed, pthread_mutex_t* mutex) {
	f->setCurrentPhase(phase);
	f->setSpeed(randomspeed); // Example speed
	f->checkviolation();

	pthread_mutex_lock(&print_mutex);
	cout << "\033[1;33m" << f->getID() << " entering " << f->getCurrentPhase() << " phase at " << f->getSpeed() << " km/h \033[0m" << endl;
	f->print();
	cout << endl;
	pthread_mutex_unlock(&print_mutex);
}

void* handleFlight(void* arg) {
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

	cout << "[ " << f->getID() << " ] Requesting runway " << runway->getRunwayID() << "\n";

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
	pthread_mutex_lock(mutex); // arrived flight locks the runway and other flights wait until the first one unlocks

	if(!runway->isOccupied()){
		cout << green << "[ " << f->getID() << " ] Runway assigned.\n" << default_text;
		runway->setOccupied(true);
	if (!runway->isOccupied()) {

		cout << "[ " << f->getID() << " ] Runway " << runway->getRunwayID() << " assigned.\n";
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

		sleep(1);
		randspeed = rand() % 2;
		simulatePhase("At Gate", f, randspeed, mutex);
	}
	else if (f->getDirection() == "East" || f->getDirection() == "West") {
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
	else { //  Runway C
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

class ATC {
private:
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
                return a->getPriority() < b->getPriority();
            }

            //If they have the same priortity, FCFS
            return a->getScheduledTime() < b->getScheduledTime();
        }
    };

    priority_queue<Flight*, vector<Flight*>, FlightComparator> arrivalQueue;
    priority_queue<Flight*, vector<Flight*>, FlightComparator> departureQueue;


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

    ~AirTrafficControl() {
        // Clean up mutexes
        pthread_mutex_destroy(&mutexA);
        pthread_mutex_destroy(&mutexB);
        pthread_mutex_destroy(&mutexC);
        pthread_mutex_destroy(&print_mutex);
        
        // No need to delete flight pointers as we're using shared_ptr
    }
	void initializeSystem() {
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

        int emergencycount = rand() % 2 + 1;
        for(int i = 0; i < emergencycount; i++)
        {
            if(!allFlights.empty())
            {
                int index = rand() % allFlights.size();
                Flight* emergencyflight = allFlights[index];

                if(emergencyflight->getPriority() != 0)
                {
                    declareEmergency(emergencyflight);
                }
            }
        }

        cout << yellow << "\nQueues before simulation:" << default_text << endl;
        testPriorityQueues();
        cout << endl;

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
        if(flight->getFlightType() == "Cargo")
        {
            return &rwyC;
        }

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
        flight->setPriority(0); // Emergency

        if(oldPriority != 0)
        {
            cout << red << "[" << flight->getID() << "]" << default_text << "Emergency Declared! Re-ordering Queues..." << endl;

            vector<Flight*> tempflights;
            while(!arrivalQueue.empty())
            {
                tempflights.push_back(arrivalQueue.top());
                arrivalQueue.pop();
            }

            for(auto f : tempflights)
            {
                arrivalQueue.push(f);
            }

            tempflights.clear();
            while(!departureQueue.empty())
            {
                tempflights.push_back(departureQueue.top());
                departureQueue.pop();
            }
            for(auto f : tempflights)
            {
                departureQueue.push(f);
            }

            Runway* emergencyrunway = allocateRunway(flight);
            {
                if(!emergencyrunway->isOccupied())
                {
                    cout << green << "[" << flight->getID() << "]" << default_text << "Emergency flight assigned to " << emergencyrunway->getRunwayID() << endl;
                }
            }
        }
    }
};

int main() {
	srand(time(0));
	ATC atc;
	atc.initializeSystem();

	// atc.addFlight("PIA", "Passenger", "North", "12:00");
	// atc.addFlight("FedEx", "Cargo", "West", "13:00");
	// // atc.addFlight("AirBlue", "Passenger", "South", "14:00");
	// atc.addFlight("PIA", "Passenger", "East", "15:00");

	atc.simulateFlights();
	return 0;
}
