#include <iostream>
#include "ATC.hpp"

int main(){
	Aircraft* pia = new Aircraft("AC-001", "Boeing 777", 300);
	Runway rwyA;
	Flight* flight = new Flight("PK-123", "Passenger", "North", 400, "Holding", &rwyA, *pia);

	sf::RenderWindow window(sf::VideoMode(1280, 720), "ATC Simulation");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        flight->draw(window);

        window.display();
    }


	// ATC atc;
	// atc.initializeSystem();
	// atc.simulateFlights();
	return 0;
}