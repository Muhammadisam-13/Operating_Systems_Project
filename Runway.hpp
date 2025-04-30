#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
using namespace sf;
using namespace std;

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