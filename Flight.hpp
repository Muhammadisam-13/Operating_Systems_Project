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
    bool runwayGiven;
    bool isActive;

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
		AVNStatus(false), aircraft(aircraft), runwayGiven(false), isActive(true)
        {
            // sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
            textureRect = new IntRect(); 
            if(flightType == "Passenger")
            {
                priority = 3;
                texture.loadFromFile("plane.png");
                if(runwayPtr->getRunwayID() == "RWY-A"){
                    *textureRect = IntRect(284, 0, 138, 170);
                }
                else if(runwayPtr->getRunwayID() == "RWY-B"){
                    *textureRect = IntRect(0, 0, 128, 170);
                }
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
                *textureRect = IntRect(0, 0, 200, 150.5); // Adjust these values
            }
            else if (flightType == "Medical") 
            {
                priority = 0;
                texture.loadFromFile("militarysprite.png");
                *textureRect = IntRect(0, 170, 100, 80); // Adjust these values
            }
            sprite.setScale(0.75, 0.75);
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
    void setAngle(bool ang){
        this->angle = ang;
    }
    void setSpriteRotation(float angle){
        sprite.setRotation(angle);
    }
    bool getAngle() const{
        return angle;
    }
    void setActive(bool act){
        this->isActive = act;
    }
    bool getActive() const{
        return isActive;
    }

    void setPath() {
        if (assignedRunwayPtr->getRunwayID() == "RWY-A")
        {
            sprite.setRotation(90);
            if (direction == "North" || direction == "South")
            {
                sprite.setPosition(-600, 350);
            }
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-B")
        {
            if (direction == "East" || direction == "West")
            {
                sprite.setPosition(1100 + rand() % 250, 650);
            }
            
        }
        else if (assignedRunwayPtr->getRunwayID() == "RWY-C")
        {
            if(currentPhase == "At Gate"){
                sprite.setPosition(1200 + rand() % 200, 750);
            }
            else if(currentPhase == "Holding"){
                sprite.setPosition(-1000 + rand() % 250, 725);
            }
        }
    }

        
    // Make sure getSprite() is implemented correctly
    Sprite& getSprite() 
    {
        return sprite;
    }
    bool isRunwayAllocated() const{
        return runwayGiven;
    }

    void setSpritePos(int x, int y){
        sprite.setPosition(x, y);
    }
    
    
    void move(sf::Clock clk = Clock()){

        Clock clk1;
        clk1.restart();
        float speedFactor = speed / 100.0f;
        float taxiSpeed = speed / 300.0f;
        int xpos = sprite.getPosition().x, ypos = sprite.getPosition().y;

        

        //  // landing or takeoff phase
        //  while(clk1.getElapsedTime().asSeconds() < 3){
        //     if(assignedRunwayPtr->getRunwayID() == "RWY-A"){     
        //         xpos += speedFactor;
        //     }
        //     else if(assignedRunwayPtr->getRunwayID() == "RWY-B"){
        //         xpos -= speedFactor;
        //     }
        //     sprite.setPosition(xpos, ypos);
        // }
       
        if(assignedRunwayPtr->getRunwayID() == "RWY-A"){     
            xpos += speedFactor;
        }
        else if(assignedRunwayPtr->getRunwayID() == "RWY-B"){
            xpos -= speedFactor;
        }

        sprite.setPosition(xpos, ypos);
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