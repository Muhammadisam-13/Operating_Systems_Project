#include <iostream>
using namespace std;

class Airline{
public:
    string name;
    string type;
    int numAircrafts;
    int numFlights;
    Airline() {}
    Airline(string name, string type, int numAircrafts, int numFlights) {
        this->name = name;
        this->type = type;
        this->numAircrafts = numAircrafts;
        this->numFlights = numFlights;
    }
    void print() const{
        cout << "Airline: " << name << endl;
        cout << "Type: " << type << endl;
        cout << "Number of aircrafts: " << numAircrafts << endl;
        cout << "Number of flights in operation: " << numFlights << endl;
    }
};


class Flight{
public:
    string type;
    char direction;
    Airline* airline;
    Flight() {}
    Flight(string type, char dir, Airline* al) {
        this->type = type;
        this->direction = dir;
        this->airline = new Airline(*al);
    }
};


int main(){
    cout << "hello world" << endl;
    return 0;
}