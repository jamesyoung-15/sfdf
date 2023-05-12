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