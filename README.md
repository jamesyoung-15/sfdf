# SFDF Project Documentation
Worked on reading sensor data from various water sensors (eg. pH, DO, etc.) through RS485 to ESP32 and sending those data to AWS through SIM7600. The ESP32 can also connect to another ESP32 through ESP-Now and send commands to it with a master-slave connection.

The main overall Arduino code/sketch is [sfdf.ino](./sfdf.ino) (or see below). It uses 3 custom libraries I wrote. More documentation for the usage of each library are inside the README of the libraries.

## Custom Libraries Written
- Library for working with ESP-Now: [ESPNowLib](./Arduino_Libraries/ESP32NowLib/)

- Library for working with SFDF-sensors: [SFDFSensorLib](./Arduino_Libraries/SFDFSensorLib/)

- Library for working with SIM7600 module with ESP32: [SIM7600AWSLib](./Arduino_Libraries/SIM7600AWSLib/)


## Main Code

``` C++
/* 
Author: Young, James Yang

This example is from Graphite Venture internship SFDF project where we read temperature sensor with Modbus, pass the data to a SIM7600 module then send it to AWS.
Also can connect and communicate to another ESP32 with ESP-Now.
The SIM7600 module can also subscribe to topics from AWS and do commands based on message. In this example, the AWS can send a command to turn the pump on and off that is controlled by another ESP32 with ESP-Now connected to this node.

Uses mix of own custom libraries and some other custom libraries like ArduinoJson, Modbus, etc. Check libraries for more detail.
*/

#include <ESP32NowLib.h>
#include <SFDFSensor.h>
#include "SIM7600_AWS.h"


// Serial 2 uses pin 16 (U2RX) and 17 (U2TX) on ESP32 Dev C Wroom, for this example for AWS
#define RXD2 16
#define TXD2 17

// Serial 1, pin 18 (U1RX) and 19 (U1TX), for this example used for sensors
#define RXD1 18
#define TXD1 19


// ModMaster Nodes, for getting data with modbus
ModbusMaster node_ec;
ModbusMaster node_ph;
ModbusMaster node_do;

// create class instance for modbus sensor nodes
sensorNodes nodes(&node_ec,&node_ph,&node_do);


// Create AWS class instance
SIM7600AWS aws(&Serial2, &Serial);


// Create ESP32 Now class node
ESP32Now espNode(1);

// bools to check connection status of each slave
bool connectionStatus1;

// millis variable for sending data every x seconds without using delay
unsigned long pervious_sent_millis = 0;
const long send_interval = 30000;  // interval to send data to AWS, in milliseconds

void setup() 
{


    // Start Serial port
    Serial.begin(115200);
    Serial.println("ESP32 + SIM7600 -> AWS Example.");

    // Start serial port to SIM7600 with RXD2 and TXD2
    Serial2.begin (115200, SERIAL_8N1, RXD2, TXD2);

    aws.disconnectAWS();
    delay(1000);
    
    // make sure you configured the module beforehand and pass the correct matching file names
    aws.configureSSL("cacert","clientcert","clientkey");
    delay(1000);

    // first parameter can be whatever name you desire, second parameter replace with your own endpoint
    aws.connectAWS("client01", "a5sswhj5ru4gy-ats.iot.us-east-2.amazonaws.com");
    delay(1000);

    // subscribe to topic, replace with your own if you like
    aws.subscribeTopic("sfdf/client01/command");

    // Start serial 1
    Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
    delay(1000);

    // Start sensor modmaster nodes
    node_ph.begin(1, Serial1);
    node_ec.begin(2, Serial1);
    node_do.begin(3, Serial1);

    // Initialize this ESP32 as ESP-Now master node with callback function passed through, make sure to define a callback function (check end of file for example)
    espNode.ESPNowStartMaster(OnDataSent);
    // Connect to another ESP32 with ESP-Now that is set to slave mode
    connectionStatus1 = espNode.addPeer("Slave 1"); // copy this line with different name for more peers

}

void loop() 
{
    // Timer for sending sensor data every send_interval seconds
    unsigned long current_millis = millis();
    if (current_millis - pervious_sent_millis >= send_interval) 
    {
        // update previous_sent_millis tracker
        pervious_sent_millis = current_millis;
        
        // example of sending some sensor datas
        double ph = nodes.readPh();
        double ec = nodes.readEC();
        double do_data = nodes.readDO();
        double temperature = nodes.readTemperature();
        // btw this function is specifically made for SFDF project that will format the data into JSON and send it as a String 
        // if you want your own custom String message format it before and use sendDataAWS function in library instead 
        aws.sendSensorData("sfdf/client01/sensor_data", ph,do_data,ec,temperature);

        // also periodically check the connection of the ESP32 slave, if not connected will attempt to repair
        connectionStatus1 = espNode.addPeer("Slave 1");
    }

    /**
      Example of receiving message from AWS and based on message relay command to another ESP32 via ESP-Now.

      In this example, the ESP32 will be checking for AWS messages and if Serial2 contains the word "response", it means that there was a message from AWS with a command
      such as "PUMPON". Example of the published AWS message for this is {response: "PUMPON"}. Once it receives AWS message with either "PUMPON" or "PUMPOFF", it will activate
      function sendESPNow.

      This function in the library is not well written with very limited application, feel free to modify it to your needs or add another function in the library.
     */
    else
    {
      // first parameter is message to look for to indicate message from AWS, second and third are commands, fourth is who to send ESPNow to ("All" or SSID name like "Slave 01"), fourth is what function to activate
      aws.checkResponseAWS("response","PUMPON", "PUMPOFF", "All", sendESPNow);
    }

}

// Example of executing a function once it receives message from AWS
void sendESPNow(String command ,String slaveName)
{
    // If passed slaveName is not "All", then send to only one peer with corresponding slaveName
    if(slaveName != "All")
    {
        espNode.sendDataSingle(command, slaveName);
    }
    // Otherwise send to all connected ESP-Now
    else
    {

        espNode.sendDataAll(command);
    }
}


// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    // print the MAC address to send to
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Sent to: "); Serial.println(macStr);
    // check status
    Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

```