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