#pragma once
#include <iostream>
#include "Flight.hpp"
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

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