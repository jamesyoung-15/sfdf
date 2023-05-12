#include "SIM7600_AWS.h"

// Serial 2 uses pin 16 (U2RX) and 17 (U2TX) on ESP32 Dev C Wroom
#define RXD2 16
#define TXD2 17

// Create class instance
SIM7600AWS aws(&Serial2, &Serial);

void setup() {
    // Start Serial port connecting to laptop
    Serial.begin(115200);
    Serial.println("ESP32 + SIM7600 -> AWS Example.");

    // Start serial port to SIM7600 with RXD2 and TXD2
    Serial2.begin (115200, SERIAL_8N1, RXD2, TXD2);


    aws.disconnectAWS();
    delay(1000);
    // make sure you configured the module beforehand and pass the correct matching file names
    aws.configureSSL("cacert","clientcert","clientkey");
    delay(1000);

    // replace with your own endpoint
    aws.connectAWS("client01", "a5sswhj5ru4gy-ats.iot.us-east-2.amazonaws.com");
    delay(1000);



  

}

void loop() {
    // Send message hello with topic name /client01 every 60 sec, replace with your own topic and message if you wish
    aws.sendDataAWS("/client01","{message: \"Hello\"}");

    // example of sending some sensor datas, use some fake data
    // double ph = 8.1;
    // double ec = 0.7;
    // double do_data = 2.4;
    // double temperature = 23;
    // aws.sendSensorData("sfdf/client01/sensor_data", ph,do_data,ec,temperature);

    delay(60000);

    /* 
     In this example, the ESP32 will be checking for AWS messages and if Serial2 contains the word "response", it means that there was a message from AWS with a command
     such as "PUMPON". Example of the published AWS message for this is {response: "PUMPON"}. Once it receives AWS message with either "PUMPON" or "PUMPOFF", it will activate
     function receivedMessage("PUMPON", "All").
     */
    aws.checkResponseAWS("response","PUMPON", "PUMPOFF", "All", receivedMessage);

}

// Example of executing a function once it receives message from AWS
void receivedMessage(String command ,String slaveName)
{
    // do something
    Serial.println("Reached here from AWS message.");
    Serial.println("Command was: " + command);
    Seral.println("Send command to: " + slaveName);
}