#include <Arduino.h>
#include <RF24.h>

const byte PIN_RADIO_INT = 2;
const byte PIN_RADIO_CS = 8;
const byte PIN_RADIO_EN = 7;

// Hardware configuration
RF24 radio(PIN_RADIO_EN, PIN_RADIO_CS);
bool reciever_role = false; // Are we the reciever? Vehicle is 'reciever', station is 'sender'

const int request_timeout_ms = 10;

// Communication pipes to use
const uint64_t pipes[2] = { 0x1122334471LL, 0x112233447CLL };

volatile bool recieved_radio_data = false; // Used to record a recieve event
char radio_message[32]; // Buffer for message (nRF24 messages are limited to 32 bytes each)
byte pipeNumber; // Stores the origin of the message (used in reciever mode)

const unsigned long SERIAL_BAUDRATE = 115200;
const bool DEBUG_MODE = false;
const bool TIMING_ROUND_TRIP = false;

struct bulkDataStruct {
  char messageType;
  char messageLength;
  uint16_t distGPS;
  uint32_t speedEncoder;
  uint32_t speedGPS;
  uint16_t rotations;
  uint16_t frontBrakeT;
  uint16_t rearBrakeT;
  uint8_t fBatt;
  uint8_t rBatt;
  uint8_t humid;
  uint8_t temp;
  uint16_t CO2;
  uint8_t fhr;
  uint8_t rhr;
  uint8_t fcad;
  uint8_t rcad;
  uint16_t fpwr;
  uint16_t rpwr;
};
bulkDataStruct dataLoad;

void flag_radio_received() {
    recieved_radio_data = true;
}

void radioSend(String outputMessage) {
    int mesLength = outputMessage.length();

    if (DEBUG_MODE) {
        Serial.print("Sending to nRF24: '");
        Serial.print(outputMessage);
        Serial.print("', length of ");
        Serial.println(mesLength);
    }
    
    if (reciever_role == true) {
        // Reciever
        // Uses the pointer to the text as a character array
        radio.writeAckPayload(pipeNumber, outputMessage.c_str(), mesLength);
    }
    else {
        // Sender, request an acknoledgement
        radio.write((outputMessage.c_str()), mesLength);
    }
}


void sendMessage (char messageType, String message, const char outputLine) {
    // Gets length byte and puts it at the front of message
    char lengthChar = message.length() + 31;

    // Sends response to appropriate line
    switch (outputLine) {
        case 'h':
            Serial.print(lengthChar);
            Serial.print(message);
            break;

        // Telemetry
        case 't':
            radioSend(String(lengthChar) + message);
            break;
    }

    if (DEBUG_MODE) {
        Serial.print(F("Sent : "));
        Serial.print(message.length());
        Serial.print(F(" - "));
        Serial.print(message);
        Serial.print(F(" on line "));
        Serial.println(outputLine);
    }
}

String readInput (char inputLine) {
    int messageLength = 0; // Stores length
    char temp[50];          // Input buffer

    String message = "";

    // Goes to the right serial line, reads message length, then the message itself
    switch (inputLine) {
        case 'h':
            while (Serial.available() == 0) {
                delayMicroseconds(10); // Wait for length character if needed
            }
            messageLength = byte(Serial.read());
            messageLength -= 31;

            Serial.readBytes(temp, messageLength);
            break;
        case 'u':
            while (Serial.available() == 0) {
                delayMicroseconds(10); // Wait for length character if needed
            }
            messageLength = Serial.read();
            messageLength -= 31;

            Serial.readBytes(temp, messageLength);
            break;

        // Telemetry
        case 't':
            messageLength = byte(radio_message[1]);
            messageLength -= 31;

            for (byte i = 0; i < messageLength; i++) {
                temp[i] = radio_message[i + 2];
            }
            break;
    }

    // Copy message to string and trim the useless end
    message = String(temp);
    message.remove(messageLength, 50 - messageLength);

    if (DEBUG_MODE) {
        Serial.print(F("Recieved: "));
        Serial.print(messageLength);
        Serial.print(F(" - "));
        Serial.print(message);
        Serial.print(F(" on line "));
        Serial.println(inputLine);
    }

    return (message);
}

void processData (const char line) {
  // Similar to the fuction on the bike but altered to suit the pass through nature of this program

  /* Gets in a value for which line to process, then processes it.
     Requests characters are lowercase, setting is uppercase
     a - Heart rate (front) (BPM)
     b - Heart rate (rear) (BPM)
     c - Cadence (front) (RPM)
     d - Cadence (rear) (RPM)
     e - Power (front) (W)
     f - Power (rear) (W)
     i - Front battery %
     j - Rear battery %
     k - Backup system battery %
     h - Humidity (R.H.%)
     t - Temperature (C * 2 + 50)
     s - Speed (km/h)
     q - Distance (number of rotations)
     l - Latitude (degrees)
     m - Longitude (degrees)
     n - Altitude (m)
     o - GPS speed (km/h)
     p - GPS distance (km)
     u - Starting longitude
     v - Starting latitude
     y - Testing byte
     z - Testing byte
  */
    char frameType = 0; // character to store data frame type

    // Gets frame type from specific line
    switch (line) {
        case 'h':
            frameType = Serial.read();
            break;
        case 'u':
            frameType = Serial.read();
            break;

        // Telemetry
        case 't':
            frameType = radio_message[0];
            break;
    }

    if (DEBUG_MODE) {
        Serial.print(F("Frame type : "));
        Serial.print(frameType);
        Serial.print(F(" on line "));
        Serial.println(line);
    }

    // Process data depending on type
    String returnMessage = ""; // Stores return message
    bool request; // Used to store if the message is a request (need a respose sent back on the same line)

    if ((frameType >= 'a') && (frameType <= '}')) {

        // Request type
        // The data will be stored to "returnMessage"
        request = true;
        if (line == 't') {
            // Radio to serial
            Serial.print(radio_message); // Send the request to the serial line
        
            unsigned long timeoutMark = millis() + request_timeout_ms;
        
            while (!Serial.available() && (timeoutMark > millis())) {
                // Kill time until either the timeout is reached or a meesage is recieved
                delayMicroseconds(100);
            }
        
            // Read in the data
            if (Serial.available()) {
                byte messageLength = byte(Serial.read()); // Get length
                returnMessage = char(messageLength); // Store length to pass on
                messageLength -= 31;
        
                char temp[messageLength];
                Serial.readBytes(temp, messageLength);
                returnMessage = returnMessage + temp; // Append data to length char for return
            }
        }
        else {
            // Serial to telemetry
            unsigned int timeoutMark;

            // Need to send request twice since the result will be in the acknowledge packet that follows
            // a request, so the first one is sent to prepare a response. The second request collects it.
            radioSend(String(frameType));
            
            radioSend(String(frameType));

            // Read in the data
            if (recieved_radio_data) {
                recieved_radio_data = false; // Reset flag

                if (radio_message[0] == '[') {
                    // Bulk transfer

                    memcpy(&dataLoad, radio_message, sizeof(dataLoad));

                    if (DEBUG_MODE) {
                        Serial.println("");
                        Serial.println(dataLoad.messageType);
                        Serial.println(dataLoad.messageLength);
                        Serial.println("Speeds:");
                        Serial.println(dataLoad.speedEncoder);
                        Serial.println(dataLoad.speedGPS);
                        Serial.println("Dist:");
                        Serial.println(dataLoad.rotations);
                        Serial.println(dataLoad.distGPS);
                        Serial.println("Brake Temps:");
                        Serial.println(dataLoad.frontBrakeT);
                        Serial.println(dataLoad.rearBrakeT);
                        Serial.println("Batteries:");
                        Serial.println(dataLoad.fBatt);
                        Serial.println(dataLoad.rBatt);
                        Serial.println("Atmosphere:");
                        Serial.println(dataLoad.humid);
                        Serial.println(dataLoad.temp);
                        Serial.println(dataLoad.CO2);
                        Serial.println("ANT:");
                        Serial.println(dataLoad.fhr);
                        Serial.println(dataLoad.rhr);
                        Serial.println(dataLoad.fcad);
                        Serial.println(dataLoad.rcad);
                        Serial.println(dataLoad.fpwr);
                        Serial.println(dataLoad.rpwr);
                    }

                    char * messingAround = (char *)&dataLoad;
                    for (byte i = 0; i < sizeof(dataLoad); i++) {
                        Serial.write(messingAround[i]);
                    }
                    return;
                }

                // Not the radio message will be prefixed with the length character
                returnMessage = radio_message;
                returnMessage.remove(0, 2); // Remove type and length char
            }
        }
    }
    else if ((frameType >= 'A') && (frameType <= ']')) {
        // Sending data
        // Send the type char along with the data associated
        request = false;
    
        // Get entire data frame
        String temp = readInput(line);
        char lengthChar = char(temp.length() + 31);
        temp = lengthChar + temp;
        temp = frameType + temp;
    
        if (line == 't') Serial.print(temp); // Radio to serial
        else radioSend(temp); // Serial to radio
    }

    // Sends return if request
    if (request) {
        if (returnMessage.length() == 0) return;
        sendMessage(frameType - 32, returnMessage, line);
    }
    if (DEBUG_MODE) Serial.println("DONE PROCESSING");
}

void setupRadio() {
    // Start radio system
    radio.begin();

    // Check for chip
    if (radio.isChipConnected() == false) {
        if (DEBUG_MODE) Serial.println("!!! nRF24L01 NOT DETECTED !!!");
        recieved_radio_data = false; // Ensure this is false
        return;
    }

    // Set up the radio if present
    radio.setPALevel(RF24_PA_MAX);          // Low power, raise if a decoupling capacitor is added
    radio.setDataRate(RF24_250KBPS);
    radio.setAutoAck(1);                    // Ensure autoACK is disabled
    radio.enableAckPayload();               // Allow optional ack payloads
    radio.setRetries(10, 15);                // Smallest time between retries, max no. of retries

    radio.enableDynamicPayloads();
    radio.setChannel(100);

    if (reciever_role == true) {
        // Reciever
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1, pipes[0]);
        radio.startListening();
    }
    else {
        // Sender
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1, pipes[1]);
        radio.stopListening();
    }
    radio.setStatusFlags(RF24_RX_DR);                 // Only toggles interrupt (falling edge/LOW) when recieving data
    //radio.printDetails();
    
    // Interrupts on recieving a message
    pinMode(PIN_RADIO_INT, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_RADIO_INT), flag_radio_received, FALLING); // Set flagging interrupt
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    // Set up the serial lines
    Serial.begin(SERIAL_BAUDRATE);
    
    if (DEBUG_MODE) {
        // Wait for debugging connection, assumed to be over USB
        while (!Serial) {
            delay(100);
        }
        Serial.println("DEBUGGING MODE ACTIVE");
    }

    setupRadio();

    if (DEBUG_MODE) {
        Serial.println("GO TIME");
    }
}

void loop() {
    static unsigned long startTime, endTime;
    
    if (Serial.available()) {
        if (TIMING_ROUND_TRIP) startTime = micros();
        processData('h');

        if (TIMING_ROUND_TRIP) {
            endTime = micros();
            Serial.print("\nRound trip time: ");
            Serial.print(endTime - startTime);
            Serial.println(" us");
        }
    }

    if (recieved_radio_data) {
        memset(radio_message, 0, sizeof(radio_message)); // Clear buffer
    
        // Start by getting where the message is from
        radio.available(&pipeNumber); // Get pipe the message we recieved on in the event a reply is needed

        // Read the message. Reads both messages and acknoledgements. 
        int messageLen = radio.getDynamicPayloadSize();
        radio.read(&radio_message, messageLen);

        radio.clearStatusFlags(); // Clear interrupt on radio
        recieved_radio_data = false; // Reset flag

        if (DEBUG_MODE) {
            Serial.print("Recieved on nRF24: '");
            Serial.print(radio_message);
            Serial.print("'. Length of ");
            Serial.println(messageLen);
        }

        processData('t');
    }
    delay(10);
}
