#include <iostream>
#include <SFML/Graphics.hpp>
#include "ATC.hpp"
using namespace sf;

int main(){
	// ATC atc;
	// atc.initializeSystem();
	// atc.simulateFlights();

    ATC atc;
    atc.run();

	return 0;
}