/* 
Main reference: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/espnow.html 

  Flow: Master
  Step 1 : ESPNow Init on Master and set it in STA mode
  Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
  Step 3 : Once found, add Slave as peer
  Step 4 : Register for send callback
  Step 5 : Start Transmitting data from Master to Slave

  Flow: Slave
  Step 1 : ESPNow Init on Slave
  Step 2 : Update the SSID of Slave with a prefix of `slave`
  Step 3 : Set Slave in AP mode
  Step 4 : Register for receive callback and wait for data
  Step 5 : Once data arrives, print it in the serial monitor

*/

#include "ESP32NowLib.h"


ESP32Now::ESP32Now(int channel):channel(channel) {}


void ESP32Now::ESPNowStartMaster(void (&onDataSent)(const uint8_t *mac_addr, esp_now_send_status_t status))
{
      // Set ESP32 as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // use only one channel
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    // Init ESPNow with a fallback logic
    InitESPNow();

    // Callback function
    esp_now_register_send_cb(onDataSent);
}

void ESP32Now::ESPNowStartSlave(String ssid, void (&onDataRec)(const uint8_t *mac_addr, const uint8_t *data, int data_len))
{
    slaveName = ssid;
    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(ssid, "Slave_1_Password", channel, 0);


    if (!result) 
    {
      Serial.println("AP Config failed.");
    } 
    else 
    {
      Serial.println("AP Config Success. Broadcasting with AP: " + String(ssid));
      Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
    }

    InitESPNow();

     // Callback function
    esp_now_register_recv_cb(onDataRec);
}

void ESP32Now::InitESPNow() 
{
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}


bool ESP32Now::addPeer(String name) 
{
    uint8_t macAddr[6] = {0};
    bool slaveFound = false;
    Serial.println("");

    // Scan network
    int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, channel); // Scan only on one channel
    if (scanResults == 0) 
    {
      Serial.println("No WiFi devices in AP Mode found");
    } 
    else 
    {
      // Check scan results
      // Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
      for (int i = 0; i < scanResults; ++i) 
      {
        // Print SSID and RSSI for each device found
        String SSID = WiFi.SSID(i);
        int32_t RSSI = WiFi.RSSI(i);
        String BSSIDstr = WiFi.BSSIDstr(i);

        //   Serial.print(i + 1);
        //   Serial.print(": ");
        //   Serial.print(SSID);
        //   Serial.print(" (");
        //   Serial.print(RSSI);
        //   Serial.print(")");
        //   Serial.println("");
      
        delay(10);
        // Check if the current device matches name
        if (SSID.indexOf(name) == 0) {
          
          // SSID of interest
          Serial.println("Found " + name);
          Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
          
          // Get BSSID => Mac Address of the Slave
          int mac[6];
          if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) 
          {
            for (int j = 0; j < 6; ++j ) 
            {
                macAddr[j] = (uint8_t) mac[j];
            }
          }
          bool exists = esp_now_is_peer_exist(macAddr);
          if(!exists)
          {
              // register peer
              slave.channel = channel; // pick a channel
              slave.encrypt = 0; // no encryption
              // add mac address to peer list
              memcpy(slave.peer_addr, macAddr, 6);
              if (esp_now_add_peer(&slave) != ESP_OK)
              {
                  Serial.println("Failed to add peer");
              }
              else
              {
                slaveFound = 1;
                Serial.println("Slave connected.");
              }
          }
          else 
          {
              Serial.println("Already connected");
              slaveFound = 1;
              break;
          }

          // we are planning to have only one slave in this example;
          // Hence, break after we find one, to be a bit efficient
          break;
        }
      }
    }

      // clean up ram
    WiFi.scanDelete();

    return slaveFound;

}


bool ESP32Now::checkPeer(const uint8_t* peerAddr)
{
    if (slave.channel == channel) 
    {
      // if (DELETEBEFOREPAIR) {
      //   deletePeer();
      // }

      Serial.print("Slave Status: ");
      // check if the peer exists
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if ( addStatus == ESP_ERR_ESPNOW_EXIST) 
      {
        // Slave already paired.
        Serial.println("Already Paired");
        return true;
      } 
      else 
      {
        Serial.println("Not connected");
        return false;
      } 
    }
    return false;
}

void ESP32Now::deletePeer(const uint8_t* addr) 
{
    esp_err_t delStatus = esp_now_del_peer(addr);
    Serial.print("Slave Delete Status: ");
    if (delStatus == ESP_OK) {
      // Delete success
      Serial.println("Success");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW Not Init");
    } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
}


void ESP32Now::sendDataAll(String data) 
{

    Serial.print("Sending: "); Serial.println(data);
    esp_err_t result = esp_now_send(0, (uint8_t*)&data, sizeof(data)); // first param null means send to all connected addressses
    
    
    
    Serial.print("Send Status: ");
    if (result == ESP_OK) {
      Serial.println("Success");
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
}

void ESP32Now::sendDataSingle(String data, String name)
{
    uint8_t macAddr[6] = {0};
    bool slaveFound = false;
    Serial.println("");

    // Scan network
    int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, channel); // Scan only on one channel
    if (scanResults == 0) 
    {
      Serial.println("No WiFi devices in AP Mode found");
      WiFi.scanDelete();
      return;
    } 
    else 
    {
      // Check scan results
      // Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
      for (int i = 0; i < scanResults; ++i) 
      {
        // Print SSID and RSSI for each device found
        String SSID = WiFi.SSID(i);
        int32_t RSSI = WiFi.RSSI(i);
        String BSSIDstr = WiFi.BSSIDstr(i);

        //   Serial.print(i + 1);
        //   Serial.print(": ");
        //   Serial.print(SSID);
        //   Serial.print(" (");
        //   Serial.print(RSSI);
        //   Serial.print(")");
        //   Serial.println("");
      
        delay(10);
        // Check if the current device matches name
        if (SSID.indexOf(name) == 0) {
          
          // SSID of interest
          Serial.println("Found " + name);
          Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
          
          // Get BSSID => Mac Address of the Slave
          int mac[6];
          if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) 
          {
            for (int j = 0; j < 6; ++j ) 
            {
                macAddr[j] = (uint8_t) mac[j];
            }
          }

          // we are planning to have only one slave in this example;
          // Hence, break after we find one, to be a bit efficient
          break;
        }
      }
    }

    if(macAddr!=nullptr)
    {

      Serial.print("Sending: "); Serial.println(data);
      // send to specified address
      esp_err_t result = esp_now_send(macAddr, (uint8_t*)&data, sizeof(data)); 


      Serial.print("Send Status: ");
      if (result == ESP_OK) {
        Serial.println("Success");
      } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW not Init.");
      } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
      } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("Internal Error");
      } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
      } else {
        Serial.println("Not sure what happened");
      }
    }
    WiFi.scanDelete();
}

void ESP32Now::printMacAddress(String name)
{
    int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, channel); // Scan only on one channel

    Serial.println("");
    if (scanResults == 0) 
    {
      Serial.println("No WiFi devices in AP Mode found");
    } 
    else 
    {
      // Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");

      for (int i = 0; i < scanResults; ++i) {
        // Print SSID and RSSI for each device found
        String SSID = WiFi.SSID(i);
        int32_t RSSI = WiFi.RSSI(i);
        String BSSIDstr = WiFi.BSSIDstr(i);

        //   Serial.print(i + 1);
        //   Serial.print(": ");
        //   Serial.print(SSID);
        //   Serial.print(" (");
        //   Serial.print(RSSI);
        //   Serial.print(")");
        //   Serial.println("");
      
        delay(10);
        // Check if the current device matches name
        if (SSID.indexOf(name) == 0) 
        {   
          // SSID of interest
          Serial.println("Slave name: "+ name);
          Serial.println("Slave mac address: " + BSSIDstr); 
          
        }
      }
    }
}


