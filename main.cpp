#include <iostream>
#include <string>
#include <vector>
using namespace std;

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
		cout << "Flight ID: " << flightID << endl;
		cout << "Flight Type: " << flightType << endl;
		cout << "Direction: " << direction << endl;
		cout << "Speed: " << speed << endl;
		cout << "Current Phase: " << currentPhase << endl;
		cout << "AVN Status: " << (AVNStatus ? "Active" : "Inactive") << endl;
		cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
		cout << "--- Aircraft Details ---\n";
		aircraft.print();
	}

	void checkviolation() {
		if (assignedRunwayPtr->getRunwayID() == "RWY-A") {
			if (currentPhase == "Holding" && speed > 600) {
				cout << "Holding Phase Violation: Speed limit exceeded. Aircraft too fast to approach. Hold 1KM around airport." << endl;
				AVNStatus = true;
			}
			else if (currentPhase == "Approach" && (speed > 290 || speed < 240)) {
				cout << "Approach Phase Violation.\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Landing" && (speed > 200 || speed < 30)) {
				cout << "Landing Phase Violation.\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Taxi" && speed > 30) {
				cout << "Taxi Phase Violation: Speed not appropriate for taxi\n";
				AVNStatus = true;
			}
			else if (currentPhase == "At Gate" && speed > 10) {
				cout << "At Gate Phase Violation: Should be stationary.\n";
				AVNStatus = true;
			}
		}
		else if (assignedRunwayPtr->getRunwayID() == "RWY-B") {
			if (currentPhase == "At Gate" && speed > 10) {
				cout << "At Gate Phase Violation: Should be stationary.\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Taxi" && speed > 30) {
				cout << "Taxi Phase Violation: Speed not appropriate for taxi\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Takeoff Roll" && (speed > 290)) {
				cout << "Takeoff Roll Phase Violation: Accelerating too fast.\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Climb" && (speed > 463)) {
				cout << "Climb Phase Violation: Max 250 knots below 10000ft.\n";
				AVNStatus = true;
			}
			else if (currentPhase == "Cruise" && (speed > 900 || speed < 800)) {
				cout << "Cruise Phase Violation: Aircraft outside the cruise limits." << endl;
				AVNStatus = true;
			}
		}
	}

	bool getAVNStatus() const { return AVNStatus; }
	string getID() const { return flightID; }
	string getDirection() const { return direction; }
	Runway* getAssignedRunwayPtr() const { return assignedRunwayPtr; }
};

class Airline {
	string name;
	string type;
	int numAircrafts;
	int numFlights;
	vector<Flight> flights;

public:
	Airline() : numAircrafts(0), numFlights(0) {}

	Airline(string name, string type, int numAircrafts, int numFlights)
		: name(name), type(type), numAircrafts(numAircrafts), numFlights(numFlights) {}

	void addFlight(const Flight& flight) {
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
			f.print();
		}
	}
};

int main() {
	Runway rwyA("RWY-A", false);
	Runway rwyB("RWY-B", false);

	vector<Aircraft> aircrafts = {
		{"AC-001", "Boeing 777", 300}, {"AC-002", "Airbus A320", 180},
		{"AC-003", "Boeing 747", 400}, {"AC-004", "Cessna 208", 12},
		{"AC-005", "Embraer E190", 100}, {"AC-006", "Bombardier CRJ", 90},
		{"AC-007", "Airbus A350", 350}, {"AC-008", "Boeing 737", 215}
	};

	Airline airline1("PIA", "Commercial", aircrafts.size(), 0);

	cout << "--- Airline Info ---\n";
	airline1.print();

	cout << "\n--- Flight Data & Violation Check ---\n";

	vector<Flight> flights = {
		{"PK-123", "Passenger", "North", 920, "Cruise", &rwyB, aircrafts[0]},
		{"PK-456", "Passenger", "East", 260, "Approach", &rwyA, aircrafts[1]},
		{"PK-789", "Cargo", "South", 700, "Holding", &rwyA, aircrafts[2]},
		{"PK-321", "Passenger", "South", 100, "Landing", &rwyA, aircrafts[3]},
		{"PK-654", "Cargo", "East", 280, "Takeoff Roll", &rwyB, aircrafts[4]},
		{"PK-987", "Passenger", "South", 40, "Taxi", &rwyA, aircrafts[5]},
		{"PK-741", "Passenger", "West", 850, "Cruise", &rwyB, aircrafts[6]},
		{"PK-852", "Passenger", "North", 500, "Climb", &rwyB, aircrafts[7]}
	};

	for (const Flight& f : flights) {
		airline1.addFlight(f);
	}

	int totalViolations = 0;
	int runwayDirectionViolations = 0;

	for (int i = 0; i < flights.size(); ++i) {
		cout << "\n==============================";
		cout << "\n--- Flight #" << i + 1 << ": " << flights[i].getID() << " ---\n";

		// Check for AVN (phase-specific) violations
		flights[i].checkviolation();
		if (flights[i].getAVNStatus()) {
			totalViolations++;
		}

		// Check and count runway-direction violations
		bool directionViolated = flights[i].getAssignedRunwayPtr()->checkRunwayViolation(
			flights[i].getDirection(), flights[i].getID());
		if (directionViolated) {
			runwayDirectionViolations++;
		}

		// Print flight details
		flights[i].print();

		// Show summary for this flight
		cout << ">>> Flight Status: "
			<< ((flights[i].getAVNStatus() || directionViolated) ? "?? Violation Detected" : "? Clear") << endl;

		// Simulate runway occupation
		flights[i].getAssignedRunwayPtr()->setRunwayOccupied();
	}

	cout << "\n==============================";
	cout << "\n?? Total AVN Violations Detected: " << totalViolations;
	cout << "\n?? Runway Direction Violations:  " << runwayDirectionViolations;
	cout << "\n==============================\n";

	return 0;
}