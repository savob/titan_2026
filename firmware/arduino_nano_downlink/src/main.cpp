#include <Arduino.h>
#include <RF24.h>
#include "printf.h"

const byte PIN_RADIO_INT = 2;
const byte PIN_RADIO_CS = 8;
const byte PIN_RADIO_EN = 7;

// instantiate an object for the nRF24L01 transceiver
RF24 radio(PIN_RADIO_EN, PIN_RADIO_CS);
 
// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
 
// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit, 0 is RX, 1 to tx
 
// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float payload = 0.0;
 
void setup() {
 
    Serial.begin(115200);
    while (!Serial) {
        // some boards need to wait to ensure access to serial over USB
    }
    
    // initialize the transceiver on the SPI bus
    if (!radio.begin()) {
        Serial.println(F("radio hardware is not responding!!"));
        while (1) {}  // hold in infinite loop
    }
    
    const int TIMEOUT_MS = 3000;
    // To set the radioNumber via the Serial monitor on startup
    Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
    while (!Serial.available() && millis() < TIMEOUT_MS) {
        // wait for user input
    }

    if (millis() < TIMEOUT_MS) {
        char input = Serial.parseInt();
        radioNumber = input == 1;
    }
    else radioNumber = 0; // Default to RX

    Serial.print(F("radioNumber = "));
    Serial.println((int)radioNumber);
    
    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_HIGH);  // RF24_PA_MAX is default.
    
    radio.setDataRate(RF24_1MBPS);
    radio.setChannel(76);
    radio.setAddressWidth(5);
    radio.clearStatusFlags(RF24_IRQ_ALL);
    radio.stopListening(address[radioNumber]);  // put radio in TX mode
    radio.setAutoAck(true);
    radio.disableDynamicPayloads();
    radio.setRetries(4, 15);
    radio.setPayloadSize(4);
 
    radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1
    
    // additional setup specific to the node's RX role
    if (!radioNumber) {
        radio.startListening();  // put radio in RX mode
    }
    
    // For debugging info
    printf_begin();             // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    radio.printPrettyDetails(); // (larger) function that prints human readable data
    Serial.println("END OF SETUP");
}  // setup
 
void loop() {
    if (radioNumber) {
        // This device is a TX node
    
        unsigned long start_timer = micros();                // start the timer
        bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
        unsigned long end_timer = micros();                  // end the timer
     
        if (report) {
            Serial.print(F("Transmission successful! "));  // payload was delivered
            Serial.print(F("Time to transmit = "));
            Serial.print(end_timer - start_timer);  // print the timer result
            Serial.print(F(" us. Sent: "));
            Serial.println(payload);  // print payload sent
            payload += 0.01;          // increment float payload
        } else {
            Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
        }
    
        // to make this example readable in the serial monitor
        delay(1000);  // slow transmissions down by 1 second
    
    } else {
        // This device is a RX node
    
        uint8_t pipe;
        if (radio.available(&pipe)) {              // is there a payload? get the pipe number that received it
            uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
            radio.read(&payload, bytes);             // fetch payload from FIFO
            Serial.print(F("Received "));
            Serial.print(bytes);  // print the size of the payload
            Serial.print(F(" bytes on pipe "));
            Serial.print(pipe);  // print the pipe number
            Serial.print(F(": "));
            Serial.println(payload);  // print the payload's value
        }
    }
}