#include "Flight.hpp"
#include "Airline.hpp" // Full include for implementation file

void Flight::print() const {
    cout << "\033[1;36m" << "--- Flight " << flightID << " Status ---" << "\033[0m" << endl;
    
    // Check if parentAirline is not null before trying to access its methods
    if (parentAirline) {
        cout << "Airline: " << parentAirline->getName() << endl;
    } else {
        cout << "Airline: Not assigned" << endl;
    }
    
    cout << "Flight ID: " << flightID << endl;
    cout << "Flight Type: " << flightType << endl;
    cout << "Direction: " << direction << endl;
    cout << "Speed: " << speed << " knots" << endl;
    cout << "Current Phase: " << currentPhase << endl;
    cout << "AVN Status: " << (AVNStatus ? "\033[1;31mActive\033[0m" : "\033[1;32mInactive\033[0m") << endl;
    
    if (assignedRunwayPtr) {
        cout << "Assigned Runway: " << assignedRunwayPtr->getRunwayID() << endl;
    } else {
        cout << "Assigned Runway: None" << endl;
    }

    cout << "--- Aircraft Details ---\n";
    aircraft.print();
    cout << "Priority: ";
    switch(priority) {
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