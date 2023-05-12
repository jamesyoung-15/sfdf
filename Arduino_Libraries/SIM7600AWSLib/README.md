# ESP32 to SIM7600 to AWS
This is a Arduino library that helps with controlling a SIM7600 module from a ESP32. It mainly is for connecting to AWS MQTT IoT core. To use, you need to install the AWS certificates (CA certificate, client key, client certificate) to the SIM7600 module beforehand (to see how to do this, you can google it or check my [guide here](./sim7600_docs/sim7600.md)).

## Libraries Needed
- Arduino-ESP32 Library
- ArduinoJson

Arduino-ESP32 Library as this library was written for ESP32. Also I used ArduinoJson library for formatting sensor data (not really needed, you can do this outside the library and pass it as a String in the general send to AWS function).

## Usage

1. Setup the Serial connection that connects to the SIM7600 module. For my example I used Serial 2 (pin 16 (U2RX) and 17 (U2TX)). 
``` C++
// Serial 2 uses pin 16 (U2RX) and 17 (U2TX) on ESP32 Dev C Wroom
#define RXD2 16
#define TXD2 17

/* 
Other code
*/

void setup()
{
    // Start serial port to SIM7600 with RXD2 and TXD2
    Serial2.begin (115200, SERIAL_8N1, RXD2, TXD2);
}
```

2. Create class instance with paramters: debugging serial port (used Serial in my example) and the serial port connected to SIM7600 (Serial2 in my example).
``` C++
// Create class instance
SIM7600AWS aws(&Serial2, &Serial);
```

3. For connecting to AWS, first I disconnect and release any MQTT connections and clients. Then I configure the SSL to the AWS certs with the matching file names passed in parameters. Finally I connect to AWS with a client name "client01" and the endpoint to the AWS.
``` C++
void setup()
{

    aws.disconnectAWS();
    
    // make sure you configured the module beforehand and pass the correct matching file names
    aws.configureSSL("cacert","clientcert","clientkey");

    // replace with your own endpoint and own client name
    aws.connectAWS("client01", "a******.amazonaws.com");
}
```

4. For sending to AWS, this library has two functions. The function `sendDataAWS(String topic, String message)` is a general way of sending a String to AWS MQTT. The function `sendSensorData(String topic, double ph, double ec, double do_data, double temperature)` is for the SFDF project where it will be formatted as a JSON.

``` C++
void loop()
{
    // set topic to /client01, message as {message: "Hello"}
    aws.sendDataAWS("/client01","{message: \"Hello\"}");
    delay(10000);
}
```

5. To receive from AWS, first subscribe to topic then in loop use `void checkResponseAWS(String check, String command1, String command2, String slaveName, void (&func)(String,String))` function. This function is a bit messy you can modify it to your own needs. See example below or the overall sfdf.ino file for more specfic usage.

## Example
Check examples folder for the example sketch.

``` C++
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

```

## References
- To see full list of SIM7600 AT commands: [SIM7600 AT Doc](./sim7600_docs/SIM7500_SIM7600_Series_AT_Command_Manual_V2.00.pdf)
- To see how to use SIM7600 MQTT with AT commands: [SIM7600 MQTT Examples](./sim7600_docs/SIM7500_SIM7600_SIM7800%20Series_MQTT_Application%20Note_V2.00.pdf)
- My own reference for setting up AWS: [SIM7600 AWS Setup](./sim7600_docs/sim7600.md)