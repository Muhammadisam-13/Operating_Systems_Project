#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

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
