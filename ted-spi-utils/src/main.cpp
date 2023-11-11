#include <iostream>
#include <unistd.h> // for the usleep() function
#include "cs5460.h" // Assuming your class definitions are here


int main() {
    // Setup
    wrapperSetup();
    CS5460 powerMeter(10);

    // Serial.begin(9600);  // Replace Arduino serial with a simple print
    powerMeter.init();
    powerMeter.startMultiConvert();
    
    // Infinite loop
    while (true) {
        std::cout << powerMeter.getCurrent() << "\t"
                  << powerMeter.getVoltage() << "\t"
                  << powerMeter.getPower() << std::endl;

        std::cout << std::hex 
                  << powerMeter.getRawCurrent() << "\t"
                  << powerMeter.getRawVoltage() << "\t"
                  << powerMeter.getRawPower() << std::endl;

        usleep(500000);  // delay(500) equivalent in microseconds
    }

    return 0;
}
