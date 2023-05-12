# ESP32NOWLib Documentation
This is a library for working with ESP-Now with ESP32 in Arduino IDE. Currently only supports single master and multiple slaves with unicast (master sends to multiple slaves, master does not listen for messages and slaves does not send messages).

This library was tested with 3 ESP32 Dev C modules, with 1 being a master and 2 being slave nodes. Master can send same message to all or just message to one slave. Doesn't need to know the MAC address beforehand as we will set each slave with a SSID name and scan for that name instead.

Current Limitation: Currently this library only supports sending a String if you wish to send another data type like a struct feel free to modify the `sendDataAll()` and `sendDataSingle()` functions in the library.

## Dependencies
These headers are used in this library:
``` c++
#include <Arduino.h>
#include "WiFi.h"
#include <esp_now.h>
#include <esp_wifi.h>
```

These libraries are:
- [ArduinoCore-avr](https://github.com/arduino/ArduinoCore-avr)
- [WiFi Library for Arduino](https://github.com/arduino-libraries/WiFi)
- [arduino-esp32](https://github.com/espressif/arduino-esp32)

Download them with library manager with Arduino IDE or manually install them and add them to your Arduino/library folder.

## Usage
Install this library and add them to your Arduino/library folder. To see how to add custom library check [this guide](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries).

<!-- For Arduino IDE you can check if it worked by restarting Arduino IDE and checking Sketch->Include Library then you should scroll down and see this library. -->

Usage flow of this library:

1. Create an instance of this class, the passed parameter is the int number you want to set the communication channel, remember to use the same channel for master and slaves.
``` C++
ESP32Now espNode(1);
```

2. Write your own a callback function for what to do on data send (master) or on data receive (slave).
``` C++

// Master example, callback function when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    // Doesn't have to do anything, maybe do something like check delivery status (see my example sketch)
}

// Slave example, callback function to handle incoming message
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    /* 
    Example: 

    Do something with received *data, maybe like convert it back to the original sent format (eg. String, struct, or whatever)

    Then do something based on that (eg. turn on a pump if the received command is "PUMPON" or something like that)
    */

}


```

3. Start it as a master or slave (don't do it as both). Remember to pass the callback function. For slave node remember to add a unique slave name for each node (it will be it's SSID).
``` C++
// master example
espNode.ESPNowStartMaster(OnDataSent);

// slave example, first parameter is the slave name, second is callback
espNode.ESPNowStartSlave("Slave 01",OnDataRec);
```

4. For master node, connect to the slave nodes with their names. The function will return a boolean for the status. You can call this function regardless if it is already connected and you can use this as a way to periodically check the connection.
``` C++
// Adds peer with the matching slave name. Can be called even if already connected.
bool status = espNode.addPeer("Slave 01");
```

5. For master node, you can send data to a single node or send to all connected nodes. Only supports sending Strings if you wish to send another data type modify these functions.
``` C++
/* Single node */
// First param is message to send (only very short String messages), second is name/SSID to send to
espNode.sendDataSingle("HelloPeer1","Slave 01");

/* All nodes */
espNode.sendDataAll("HelloAll");
```



## Example
These are example sketches, you can find them inside the [examples folder](./examples/).

Example of master node:
``` C++
#include <ESP32NowLib.h>

// Create ESP32 Now node
ESP32Now espNode(1);

// bools to check connection status of each slave
bool connectionStatus1;
bool connectionStatus2;


void setup() 
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("ESPNow/Basic/Master Example");

    // Initialize the master node
    espNode.ESPNowStartMaster(OnDataSent);
    // connect to slaves, parameter is the slave name you set in the peer ino
    connectionStatus1 = espNode.addPeer("Slave 1"); // copy this line with different name for more peers
    connectionStatus2 = espNode.addPeer("Slave 2");
}

void loop() 
{

    /* 
    example of sending commands to all connected slaves
    if slave is connected, send the message "PUMPON" (replace with whatever String you want to send), if you would like to change data type feel free to modify function in library  
    */
    if(connectionStatus1 && connectionStatus2)
    {
      espNode.sendDataAll("PUMPON");
      delay(5000);
    }

    // example of only sending to one slave
    if(connectionStatus1)
    {
        espNode.sendDataSingle("Slave1Message", "Slave 1");
        delay(5000);
    }

    // check connections again
    connectionStatus1 = espNode.addPeer("Slave 1");
    connectionStatus2 = espNode.addPeer("Slave 2");

    // if slave1 not connected, try again
    if(!connectionStatus1)
    {
        connectionStatus1 = espNode.addPeer("Slave 1");
        delay(3000);
    }

    // if slave2 not connected, try again
    if(!connectionStatus2)
    {
        connectionStatus2 = espNode.addPeer("Slave 2");
        delay(3000);
    }
}


// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Sent to: "); Serial.println(macStr);
    Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

```

Example of slave node, you can re-use this for every slave you create and just change the slave name:

``` C++
#include "ESP32NowLib.h"



// Create ESP32 node
ESP32Now espNode(1);



void setup() {
    Serial.begin(115200);
    Serial.println("ESPNow Slave Example");

    // Set ESP32 node as slave
    espNode.ESPNowStartSlave("Slave 1", OnDataRecv);

  

}

void loop() {
  // Will be listening for messages
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) 
{
    // copy data to char array buffer, then convert to arduino string
    char buffer[ESP_NOW_MAX_DATA_LEN-1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, data_len);
    strncpy(buffer, (const char *)data, msgLen);
    // Make sure we are null terminated
    buffer[msgLen] = 0; 

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    Serial.print("Last Packet Recv from: "); Serial.println(macStr);
    Serial.print("Last Packet Recv Data: "); Serial.println(buffer);
    // Check the received command and can handle accordingly
    if(strstr(buffer,"PUMPON")!=NULL)
    {
    Serial.println("Command sent was to turn on pump");
    }
    else if(strstr(buffer,"PUMPOFF")!=NULL)
    {
    Serial.println("Command sent was to turn off pump");
    }
}
```


## References:
- [Main Arduino IDE ESP-Now Reference Used](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/espnow.html)
- [ESP-Now documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [Example of one master multiple slaves with ESP-Now](https://randomnerdtutorials.com/esp8266-esp-now-one-master-multiple-slaves/)