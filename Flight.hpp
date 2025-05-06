#pragma once
#include <iostream>
#include "Runway.hpp"
#include "Aircraft.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Colors.hpp"
#include <cmath>
#include <chrono>
#include <thread>

using namespace std;
using namespace sf;

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

    // SFML
    float angle;
    Sprite sprite;
    Texture texture;
    IntRect* textureRect;
    vector<sf::Vector2f> path;
    
public:
	Flight() : speed(0), AVNStatus(false), assignedRunwayPtr(nullptr), priority(3) {}

	Flight(string flightID, string flightType, string direction, int speed,
		string currentPhase, Runway* runwayPtr, Aircraft aircraft)
		: flightID(flightID), flightType(flightType), direction(direction),
		speed(speed), currentPhase(currentPhase), assignedRunwayPtr(runwayPtr),
		AVNStatus(false), aircraft(aircraft)
        {
            sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
            textureRect = new IntRect(); 
            if(flightType == "Passenger")
            {
                priority = 3;
                texture.loadFromFile("plane.png");
                *textureRect = IntRect(0, 0, 128, 170);
            }
            else if(flightType == "Cargo")
            {
                priority = 2;
                texture.loadFromFile("plane.png");
                *textureRect = IntRect(128, 0, 156, 170);
            }
            else if (flightType == "Military") 
            {
                priority = 1;
                texture.loadFromFile("militarysprite.png");
                *textureRect = IntRect(0, 0, 160, 80); // Adjust these values
            }
            else if (flightType == "Medical") 
            {
                priority = 0;
                texture.loadFromFile("militarysprite.png");
                *textureRect = IntRect(0, 170, 100, 80); // Adjust these values
            }
            sprite.setTexture(texture);
            sprite.setTextureRect(*textureRect);
            setPath();
        }

    ~Flight() {
        delete textureRect;
    }

    bool loadSprite(string& filename) {
        if (!texture.loadFromFile(filename)) 
        {
            return false;
        }
        sprite.setTexture(texture);
        return true;
    }

    void setPath() {
        if (assignedRunwayPtr->getRunwayID() == "RWY-A")
        {
            if (direction == "North")
            {
                path.push_back(Vector2f(617, 214));  //approx
                path.push_back(Vector2f(617, 300));
                path.push_back(Vector2f(617, 400));
            }
            else if (direction == "South")
            {
                path.push_back(Vector2f(617, 400));
                path.push_back(Vector2f(617, 300));
                path.push_back(Vector2f(617, 214));
            }
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-B")
        {
            if (direction == "East")
            {
                path.push_back(Vector2f(770, 500));
                path.push_back(Vector2f(850, 500));
                path.push_back(Vector2f(950, 500));
                path.push_back(Vector2f(1050, 500));
                path.push_back(Vector2f(1150, 500));
            }
            else if (direction == "West")
            {
                path.push_back(Vector2f(770, 500));
                path.push_back(Vector2f(670, 500));
                path.push_back(Vector2f(570, 500));
                path.push_back(Vector2f(470, 500));
                path.push_back(Vector2f(370, 500));
            }
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-C")
        {
            if (direction == "North")
            {
                path.push_back(Vector2f(250, 100));
                path.push_back(Vector2f(250, 200));
                path.push_back(Vector2f(250, 300));
                
            }
            else if (direction == "South")
            {
                path.push_back(Vector2f(250, 300));
                path.push_back(Vector2f(250, 200));
                path.push_back(Vector2f(250, 100));
            }
        }
    }

        
    // Make sure getSprite() is implemented correctly
    Sprite& getSprite() 
    {
        return sprite;
    }

    void setSpritePos(int x, int y){
        sprite.setPosition(x, y);
    }
    
    void move() {
        float speedFactor = speed / 100.0f;
        if (currentPhase != "At Gate") {
            if (path.size() > 0)
            {
                Vector2f target = path[0];
                Vector2f currentPos = sprite.getPosition();
                float dx = target.x - currentPos.x;
                float dy = target.y - currentPos.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance > speedFactor)
                {
                    angle = atan2(dy, dx) * 180 / 3.14159265;
                    sprite.setRotation(90);
                    currentPos.x += speedFactor * cos(angle * 3.14159265 / 180.0f);
                    currentPos.y += speedFactor * sin(angle * 3.14159265 / 180.0f);
                    sprite.setPosition(currentPos);
                }
                else
                {
                    sprite.setPosition(target);
                    path.erase(path.begin());
                }
            }
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