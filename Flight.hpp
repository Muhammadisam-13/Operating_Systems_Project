#pragma once
#include <iostream>
#include "Runway.hpp"
#include "Aircraft.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Colors.hpp"

using namespace std;
using namespace sf;

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

    Sprite sprite;
	Texture* texture;
    IntRect* textureRect;

public:
	Flight() : speed(0), AVNStatus(false), assignedRunwayPtr(nullptr) {}

	Flight(string flightID, string flightType, string direction, int speed,
		string currentPhase, Runway* runwayPtr, Aircraft aircraft)
		: flightID(flightID), flightType(flightType), direction(direction),
		speed(speed), currentPhase(currentPhase), assignedRunwayPtr(runwayPtr),
		AVNStatus(false), aircraft(aircraft) {
            textureRect = new IntRect(0, 0, 128, 160);
            texture = new Texture;
            texture->loadFromFile("plane.png");
            // sprite.rotate(90.0f);
            sprite.setTexture(*texture);
            sprite.setTextureRect(*textureRect);
            sprite.setScale(1, 1);
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
		cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
		cout << "--- Aircraft Details ---\n";
		aircraft.print();
	}
    void draw(RenderWindow& window) {
        window.draw(sprite);
    }
    void setPosition(int x, int y){
        sprite.setPosition(x, y);
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