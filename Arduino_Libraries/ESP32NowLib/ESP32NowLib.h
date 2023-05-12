/* Main reference: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/espnow.html  */

#ifndef ESP32NOWLIB_H
#define ESP32NOWLIB_H

 

// Arduino Libraries
#include <Arduino.h>
#include "WiFi.h"
#include <esp_now.h>
#include <esp_wifi.h>


class ESP32Now
{
    private:
        int channel;
        esp_now_peer_info_t slave;
        String slaveName;

    public:

        /**!
        * @brief Class constructor initializes channel as will be only using single channel
        * @param Wi-Fi channel to use
        */
        ESP32Now(int channel);

        /**!
        * @brief Class constructor initializes channel as will be only using single channel
        * @param Callback function that tells what to do when message is sent
        */
        void ESPNowStartMaster(void (&func)(const uint8_t *mac_addr, esp_now_send_status_t status));

        /**!
        * @brief Start a slave node
        * @param String of SSID name of slave you wish to set, 
        * @param Callback function that tells what to do when message is received
        */
        void ESPNowStartSlave(String ssid, void (&onDataRec)(const uint8_t *mac_addr, const uint8_t *data, int data_len));

        /**!
        * @brief Init ESP Now with fallback
        */
        void InitESPNow();

        /**!
        * @brief Add a slave in AP mode with matching SSID name 
        * @param String slave name to look for and add
        * @return Return true if found and connected, false if not
        */
        bool addPeer(String name);

        /**! 
        * @brief Check if the slave is already paired with the master.
        * @param peerAddr is the mac address of peer to check
        * @return Returns true if already paired, false if not
        */
        bool checkPeer(const uint8_t* peerAddr);

        /**!
        * @brief Deletes peer
        * @param addr Mac address of peer to delete
        */
        void deletePeer(const uint8_t* addr);

        /**! 
        * @brief Send data based on passed paramter to all ESP-NOW slaves if connected
        * @param data is a string of the data to be sent, change paramter to another data type if desired such as struct
        */
        void sendDataAll(String data);

        /**!
        * @brief Sends data from parameter to a single slave from passed mac address
        * @param data of message to send
        * @param Name of device to send to
        */
        void sendDataSingle(String data, String name);

        /**!
        * @brief Prints mac address with matchin SSID name
        * @param Name of device
        * @return Returns mac address with matching name
        */
        void printMacAddress(String name);

        


    

};

#endif
