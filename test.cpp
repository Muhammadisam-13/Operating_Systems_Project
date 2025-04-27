#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <atomic>
#include <iomanip>
#include <condition_variable>
using namespace std;

// Global mutex for console output to prevent garbled text
mutex consoleMutex;
// Mutexes for runways
mutex runwayMutexA;
mutex runwayMutexB;
// Condition variables for runway availability
condition_variable runwayAvailableA;
condition_variable runwayAvailableB;
// Tracking runway status
bool isRunwayAFree = true;
bool isRunwayBFree = true;

class Aircraft 
{
	string aircraftID;
	string model;
	int capacity;

public:
	// Default constructor of Aircraft
	Aircraft()
	{
		aircraftID = "";
		model = "";
		capacity = 0;
	}
	// Parameterized constructor of Aircraft
	Aircraft(string id, string model, int capacity)
	{
		aircraftID = id;
		this->model = model;
		this->capacity = capacity;
	}

	// Print function to display aircraft details
	void print() const 
	{
		cout << "Aircraft ID: " << aircraftID << ", Model: " << model << ", Capacity: " << capacity << endl;
	}

	// Getter function to retrieve aircraft ID
	string getID() const { return aircraftID; }

	// Getter for model
	string getModel() const { return model; }
};

class Runway 
{
	string runwayID;
	bool occupationstatus;

public:
	Runway()
	{
		runwayID = "";
		occupationstatus = false;
	}

	Runway(string runwayID, bool status)
	{
		this->runwayID = runwayID;
		this->occupationstatus = status;
	}

	void print() const
	{
		lock_guard<mutex> lock(consoleMutex);
		cout << "Runway ID: " << runwayID << endl;
		cout << "Occupation Status: " << (occupationstatus ? "Occupied" : "Available") << endl;
	}

	void setOccupied(bool status)
	{
		occupationstatus = status;
		if (runwayID == "RWY-A") 
		{
			isRunwayAFree = !status;
			if (!status) runwayAvailableA.notify_one();
		}
		else if (runwayID == "RWY-B") 
		{
			isRunwayBFree = !status;
			if (!status) runwayAvailableB.notify_one();
		}
	}

	bool isOccupied() const 
	{
		return occupationstatus;
	}

	string getRunwayID() const { return runwayID; }

	// Runway violation check based on Flight details
	bool checkRunwayViolation(const string& direction, const string& flightID) 
	{
		if (runwayID == "RWY-A") 
		{
			if (direction != "North" && direction != "South") 
			{
				lock_guard<mutex> lock(consoleMutex);
				cout << "? Runway " << runwayID << " is incorrectly assigned for direction: " << direction << " in flight " << flightID << endl;
				return true;
			}
		}
		else if (runwayID == "RWY-B") 
		{
			if (direction != "East" && direction != "West") 
			{
				lock_guard<mutex> lock(consoleMutex);
				cout << "? Runway " << runwayID << " is incorrectly assigned for direction: " << direction << " in flight " << flightID << endl;
				return true;
			}
		}
		return false;
	}
};

class Flight {
	string flightID;
	string flightType;
	string direction;
	int speed;
	string currentPhase;
	bool AVNStatus;
	Runway* assignedRunwayPtr;
	Aircraft aircraft;
	bool isArrival;  // True for arrival flights, false for departures
	bool completed;  // Track if flight has completed all phases

public:
	Flight() 
	{
		flightID = "";
		flightType = "";
		direction = "";
		currentPhase = "";
		aircraft = Aircraft();
		speed = 0;
		assignedRunwayPtr = nullptr;
		AVNStatus = false;
		isArrival = true;
		completed = false;
	}

	Flight(string flightID, string flightType, string direction, int speed, string currentPhase, Runway* runwayPtr, Aircraft aircraft, bool isArrival)
	{
		this->flightID = flightID;
		this->flightType = flightType;
		this->direction = direction;
		this->speed = speed;
		this->currentPhase = currentPhase;
		this->assignedRunwayPtr = runwayPtr;
		this->aircraft = aircraft;
		this->AVNStatus = false;
		this->isArrival = isArrival;
		this->completed = false;
	}

	void print() const 
	{
		lock_guard<mutex> lock(consoleMutex);
		cout << "\033[1;36m" << "--- Flight " << flightID << " Status ---" << "\033[0m" << endl;
		cout << "Flight ID: " << flightID << endl;
		cout << "Flight Type: " << flightType << endl;
		cout << "Direction: " << direction << endl;
		cout << "Speed: " << speed << " knots" << endl;
		cout << "Current Phase: " << currentPhase << endl;
		cout << "Operation Type: " << (isArrival ? "Arrival" : "Departure") << endl;
		cout << "AVN Status: " << (AVNStatus ? "\033[1;31mActive\033[0m" : "\033[1;32mInactive\033[0m") << endl;
		cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
		cout << "Aircraft: " << aircraft.getModel() << " (" << aircraft.getID() << ")" << endl;
		cout << "-----------------------------" << endl << endl << endl;
	}

	void checkviolation()
	{
		bool violation = false;
		string violationMessage;

		if (assignedRunwayPtr->getRunwayID() == "RWY-A")
		{
			if (currentPhase == "Holding" && speed > 600)
			{
				violationMessage = "Holding Phase Violation: Speed limit exceeded. Aircraft too fast to approach. Hold 1KM around airport.";
				violation = true;
			}
			else if (currentPhase == "Approach" && (speed > 290 || speed < 240)) {
				violationMessage = "Approach Phase Violation: Speed must be between 240-290 knots.";
				violation = true;
			}
			else if (currentPhase == "Landing" && (speed > 200 || speed < 30)) {
				violationMessage = "Landing Phase Violation: Speed must be between 30-200 knots.";
				violation = true;
			}
			else if (currentPhase == "Taxi" && speed > 30) {
				violationMessage = "Taxi Phase Violation: Speed not appropriate for taxi (max 30 knots)";
				violation = true;
			}
			else if (currentPhase == "At Gate" && speed > 10) {
				violationMessage = "At Gate Phase Violation: Should be near stationary (max 10 knots).";
				violation = true;
			}
		}
		else if (assignedRunwayPtr->getRunwayID() == "RWY-B") {
			if (currentPhase == "At Gate" && speed > 10) {
				violationMessage = "At Gate Phase Violation: Should be near stationary (max 10 knots).";
				violation = true;
			}
			else if (currentPhase == "Taxi" && speed > 30) {
				violationMessage = "Taxi Phase Violation: Speed not appropriate for taxi (max 30 knots)";
				violation = true;
			}
			else if (currentPhase == "Takeoff Roll" && (speed > 290)) {
				violationMessage = "Takeoff Roll Phase Violation: Accelerating too fast (max 290 knots).";
				violation = true;
			}
			else if (currentPhase == "Climb" && (speed > 463)) {
				violationMessage = "Climb Phase Violation: Max 250 knots below 10000ft (463 knots).";
				violation = true;
			}
			else if (currentPhase == "Cruise" && (speed > 900 || speed < 800)) {
				violationMessage = "Cruise Phase Violation: Aircraft outside the cruise limits (800-900 knots).";
				violation = true;
			}
		}
        
		if (violation) {
			lock_guard<mutex> lock(consoleMutex);
			cout << "\033[1;31m" << "!! " << flightID << " VIOLATION: " << violationMessage << "\033[0m" << endl;
			AVNStatus = true;
		}
	}

	void setSpeed(int newSpeed) {
		speed = newSpeed;
	}

	void setPhase(string newPhase) {
		currentPhase = newPhase;
	}

	bool getAVNStatus() const { return AVNStatus; }
	string getID() const { return flightID; }
	string getDirection() const { return direction; }
	Runway* getAssignedRunwayPtr() const { return assignedRunwayPtr; }
	string getCurrentPhase() const { return currentPhase; }
	bool isCompleted() const { return completed; }
	void setCompleted(bool status) { completed = status; }
	bool getIsArrival() const { return isArrival; }
};

class Airline 
{
	string name;
	string type;
	int numAircrafts;
	int numFlights;
	vector<Flight> flights;
	vector<Runway> runways;

public:
	Airline()
	{
		name = "";
		type = "";
		numAircrafts = 0;
		numFlights = 0;
	}

	Airline(string name, string type, int numAircrafts, int numFlights)
	{
		this->name = name;
		this->type = type;
		this->numAircrafts = numAircrafts;
		this->numFlights = numFlights;
	}

	void addRunway(const Runway& runway) 
	{
		runways.push_back(runway);
	}

	void addFlight(const Flight& flight) 
	{
		flights.push_back(flight);
		numFlights = flights.size();
	}

	void print() const 
	{
		lock_guard<mutex> lock(consoleMutex);
		cout << "Airline: " << name << endl;
		cout << "Type: " << type << endl;
		cout << "Number of aircrafts: " << numAircrafts << endl;
		cout << "Number of flights in operation: " << numFlights << endl;
	}

	string getName() const { return name; }
};

class AirTrafficControl
{
	vector<Aircraft>& aircrafts;
	vector<Runway>& runways;
	vector<Airline>& airlines;
	atomic<int> flightCounter{ 1 }; // Atomic counter for flight IDs
	mt19937 rng;  // Random number generator

public:
	AirTrafficControl(vector<Aircraft>& aircrafts, vector<Runway>& runways, vector<Airline>& airlines)
		: aircrafts(aircrafts), runways(runways), airlines(airlines) {
		// Seed the random number generator
		random_device rd;
		rng.seed(rd());
	}

	Flight createFlight(Airline& airline, string type, string direction, bool isArrival)
	{
		string flightID = airline.getName().substr(0, 2) + "-" + to_string(flightCounter++);

		// Select a random aircraft
		uniform_int_distribution<int> aircraftDist(0, aircrafts.size() - 1);
		Aircraft aircraft = aircrafts[aircraftDist(rng)];

		// Assign runway based on direction
		Runway* assignedRunway = assignRunway(direction);
		if (!assignedRunway) {
			lock_guard<mutex> lock(consoleMutex);
			cout << "!! No valid runway for direction: " << direction << " - Flight " << flightID << " aborted.\n";
			return Flight();
		}

		// Initial phase and speed based on arrival/departure
		string initialPhase = isArrival ? "Cruise" : "At Gate";
		int initialSpeed = isArrival ? getAppropriateSpeed("Cruise") : getAppropriateSpeed("At Gate");

		Flight newFlight(flightID, type, direction, initialSpeed, initialPhase, assignedRunway, aircraft, isArrival);
		airline.addFlight(newFlight);

		return newFlight;
	}

	void simulateFlight(Airline& airline, string type, string direction, bool isArrival = true)
	{
		Flight flight = createFlight(airline, type, direction, isArrival);
		if (flight.getID() != "")
		{
			// Start a new thread for this flight
			thread flightThread(&AirTrafficControl::processFlight, this, flight);
			flightThread.detach(); // Let the flight run in the background
		}
	}


private:
	Runway* assignRunway(const string& direction)
	{
		for (auto& r : runways)
		{
			if ((r.getRunwayID() == "RWY-A" && (direction == "North" || direction == "South")) ||
				(r.getRunwayID() == "RWY-B" && (direction == "East" || direction == "West"))) {
				return &r;
			}
		}
		return nullptr;
	}

	int getAppropriateSpeed(const string& phase) {
		// Return appropriate speed range for each flight phase
		uniform_int_distribution<int> dist;

		if (phase == "Cruise") {
			dist = uniform_int_distribution<int>(800, 900);
		}
		else if (phase == "Approach") {
			dist = uniform_int_distribution<int>(240, 290);
		}
		else if (phase == "Holding") {
			dist = uniform_int_distribution<int>(500, 600);
		}
		else if (phase == "Landing") {
			dist = uniform_int_distribution<int>(30, 200);
		}
		else if (phase == "Taxi") {
			dist = uniform_int_distribution<int>(10, 30);
		}
		else if (phase == "Takeoff Roll") {
			dist = uniform_int_distribution<int>(200, 290);
		}
		else if (phase == "Climb") {
			dist = uniform_int_distribution<int>(350, 463);
		}
		else if (phase == "At Gate") {
			dist = uniform_int_distribution<int>(0, 10);
		}
		else {
			dist = uniform_int_distribution<int>(0, 10); // Default
		}

		return dist(rng);
	}

	void processFlight(Flight flight)
	{
		// Flight phase sequences
		vector<string> arrivalPhases = { "Cruise", "Approach", "Holding", "Landing", "Taxi", "At Gate" };
		vector<string> departurePhases = { "At Gate", "Taxi", "Takeoff Roll", "Climb", "Cruise" };

		vector<string>& phases = flight.getIsArrival() ? arrivalPhases : departurePhases;

		// Current phase index in the sequence
		int phaseIndex = 0;

		// Run through all flight phases
		while (phaseIndex < phases.size()) {
			string currentPhase = phases[phaseIndex];

			// Set the speed appropriate for this phase
			int appropriateSpeed = getAppropriateSpeed(currentPhase);

			// Output the phase change
			{
				lock_guard<mutex> lock(consoleMutex);
				cout << "\033[1;33m" << flight.getID() << " entering " << currentPhase << " phase at " << appropriateSpeed << " knots\033[0m" << endl;
			}

			flight.setPhase(currentPhase);
			flight.setSpeed(appropriateSpeed);
			flight.checkviolation();
			flight.print();

			// If the flight needs runway access (Landing or Takeoff)
			if (currentPhase == "Landing" || currentPhase == "Takeoff Roll") {
				Runway* runway = flight.getAssignedRunwayPtr();

				// Acquire runway mutex
				if (runway->getRunwayID() == "RWY-A") {
					unique_lock<mutex> lock(runwayMutexA);
					// Wait for runway to be free
					runwayAvailableA.wait(lock, [] { return isRunwayAFree; });

					isRunwayAFree = false;
					runway->setOccupied(true);

					// Announce runway acquisition
					{
						lock_guard<mutex> consoleLock(consoleMutex);
						cout << "\033[1;35m" << flight.getID() << " has acquired " << runway->getRunwayID() << " for " << currentPhase << "\033[0m" << endl;
					}

					// Simulate runway use
					this_thread::sleep_for(chrono::milliseconds(1500));

					// Release runway
					runway->setOccupied(false);
					isRunwayAFree = true;
					lock.unlock();
					runwayAvailableA.notify_one();

					{
						lock_guard<mutex> consoleLock(consoleMutex);
						cout << "\033[1;32m" << flight.getID() << " has released " << runway->getRunwayID() << "\033[0m" << endl;
					}
				}
				else if (runway->getRunwayID() == "RWY-B") {
					unique_lock<mutex> lock(runwayMutexB);
					// Wait for runway to be free
					runwayAvailableB.wait(lock, [] { return isRunwayBFree; });

					isRunwayBFree = false;
					runway->setOccupied(true);

					// Announce runway acquisition
					{
						lock_guard<mutex> consoleLock(consoleMutex);
						cout << "\033[1;35m" << flight.getID() << " has acquired " << runway->getRunwayID() << " for " << currentPhase << "\033[0m" << endl;
					}

					// Simulate runway use
					this_thread::sleep_for(chrono::milliseconds(1500));

					// Release runway
					runway->setOccupied(false);
					isRunwayBFree = true;
					lock.unlock();
					runwayAvailableB.notify_one();

					{
						lock_guard<mutex> consoleLock(consoleMutex);
						cout << "\033[1;32m" << flight.getID() << " has released " << runway->getRunwayID() << "\033[0m" << endl;
					}
				}
			}
			else {
				// For non-runway phases, just simulate the time passing
				this_thread::sleep_for(chrono::milliseconds(1000));
			}

			// Move to next phase if no violations
			if (!flight.getAVNStatus()) {
				phaseIndex++;
			}
			else {
				// If there's a violation, hold at current phase until resolved
				// In a real system, this would involve controller intervention
				this_thread::sleep_for(chrono::milliseconds(500));

				// Simulate correction of violation
				{
					lock_guard<mutex> lock(consoleMutex);
					cout << "\033[1;34m" << flight.getID() << " is correcting violation in " << currentPhase << " phase\033[0m" << endl;
				}

				// Correct speed for this phase and try again
				appropriateSpeed = getAppropriateSpeed(currentPhase);
				flight.setSpeed(appropriateSpeed);
				flight.checkviolation();

				// If still violated, try again in next iteration
			}
		}

		// Flight completed all phases
		{
			lock_guard<mutex> lock(consoleMutex);
			cout << "\033[1;32m" << flight.getID() << " has completed all phases. Flight operation concluded.\033[0m" << endl;
		}

		flight.setCompleted(true);
	}
};

int main() {
	// Setup runways
	Runway rwyA("RWY-A", false);
	Runway rwyB("RWY-B", false);
	vector<Runway> allRunways = { rwyA, rwyB };

	// Aircraft list
	vector<Aircraft> aircrafts = {
		{"AC-001", "Boeing 777", 300}, {"AC-002", "Airbus A320", 180},
		{"AC-003", "Boeing 747", 400}, {"AC-004", "Cessna 208", 12},
		{"AC-005", "Embraer E190", 100}, {"AC-006", "Bombardier CRJ", 90},
		{"AC-007", "Airbus A350", 350}, {"AC-008", "Boeing 737", 215}
	};

	// Create multiple airlines
	Airline pia("PIA", "Commercial", aircrafts.size(), 0);
	Airline emirates("Emirates", "Commercial", aircrafts.size(), 0);

	// Assign runways to each airline
	pia.addRunway(rwyA);
	pia.addRunway(rwyB);
	emirates.addRunway(rwyA);
	emirates.addRunway(rwyB);

	// Vector of airlines
	vector<Airline> airlines = { pia, emirates };

	// Print airline info
	cout << "=== FLIGHT SIMULATION WITH THREADS AND MUTEXES ===" << endl;
	cout << "--- Airline Info ---\n";
	for (const Airline& a : airlines) {
		a.print();
		cout << "\n";
	}

	// Print runway info
	cout << "--- Runway Info ---\n";
	for (const Runway& r : allRunways) {
		r.print();
		cout << "\n";
	}

	// Create flight simulator
	AirTrafficControl simulator(aircrafts, allRunways, airlines);

	// Direction and type options
	vector<string> types = { "Passenger", "Cargo" };
	vector<string> directions = { "North", "South", "East", "West" };

	// Seed random for different operations each run
	srand(time(0));

	cout << "\n=== STARTING FLIGHT OPERATIONS ===" << endl;

	// Create 2 arrival and 2 departure flights for each airline
	for (Airline& airline : airlines) {
		for (int i = 0; i < 2; ++i) {  // 2 arrivals
			string type = types[rand() % types.size()];
			string dir = directions[rand() % directions.size()];

			// Simulate arrival flight (true)
			simulator.simulateFlight(airline, type, dir, true);

			// Add small delay between flight creations
			this_thread::sleep_for(chrono::milliseconds(300));
		}

		for (int i = 0; i < 2; ++i) {  // 2 departures
			string type = types[rand() % types.size()];
			string dir = directions[rand() % directions.size()];

			// Simulate departure flight (false)
			simulator.simulateFlight(airline, type, dir, false);

			// Add small delay between flight creations
			this_thread::sleep_for(chrono::milliseconds(300));
		}
	}

	// Wait for all flights to complete processing
	cout << "\nAll flights initialized. Waiting for completion..." << endl;
	this_thread::sleep_for(chrono::seconds(30));  // Allow enough time for flights to complete

	cout << "\n=== SIMULATION COMPLETE ===" << endl;

	return 0;
}