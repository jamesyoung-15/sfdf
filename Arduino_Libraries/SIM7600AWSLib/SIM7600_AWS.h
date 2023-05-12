#ifndef SIM7600_AWS_H
#define SIM7600_AWS_H

// Arduino Libraries
#include <Arduino.h>
#include <ArduinoJson.h>

class SIM7600AWS
{
    private:
        String subTopic;
        String pubTopic;
        Stream* sim7600Port; // The serial port that connects to AWS (eg. Serial2)
        Stream* printSerialPort; // The serial port to view response (eg. Serial)
        unsigned int baudRate;
        bool is_sending_aws = false; // 
        bool is_receiving_aws = false; 
        

    public:

        /**!
         * @brief Constructor that sets Serial port and baudrate of serial port
         * @param Serial port used to connect to SIM7600 (eg. Serial2)
         * @param Baudrate (eg. 115200)
         */
        SIM7600AWS(Stream *simPort, Stream *printPort);

        /**!  
         * @brief Test function, send AT command and print a response to Serial
         * @param String of AT command to send 
        */
       void testSim(String commnad);


        /**!
         * @brief Configures the SSL context, authenitciation mode, relevant certficates and keys. 
         * Assumes SIM7600 was already configured with certificates downloaded.
         * @param The CA certificate file name eg. if your cacert was set to cacert.pem, pass "cacert" to this parameter
         * @param The client certificate file name eg. if your client cert was set to clientcert.pem, pass "clientcert" to this parameter
         * @param The client key file name eg. if your client key was set to clientkey.pem, pass "clientkey" to this parameter
         */
        void configureSSL(String cacert, String clientcert, String clientkey);

        /**!
         * @brief Connects to AWS endpoint with MQTT
         * @param The clientName is the name to set this device
         * @param The awsEndpoint is the url link of the endpoint (eg. )
         */
        void connectAWS(String clientName, String awsEndpoint);

        /**!
         * @brief Subscribe to MQTT topic from AWS 
         * @param String of topic name to subscribe to
        */
        void subscribeTopic(String topic);

        /**!
         * @brief Sends a message to AWS with given topic
         * @param Publish topic of message
         * @param Message content
         */
        void sendDataAWS(String topic, String message);

        /**!
         * @brief Use this function for sending water sensor data, will format the data into JSON and send to AWS. Change parameters and JSON data if you want to add more or less data to send. 
         * @param Publish topic name
         * @param pH value
         * @param EC value
         * @param DO value
         * 
        */
        void sendSensorData(String topic, double ph, double ec, double do_data, double temperature);

        /**!
         * @brief Gets the string of current time in UTC+8 from SIM7600
         * @return Returns string of current time in UTC+8 in format of “YY/MM/DD,HH:MM:SS” (eg. 23/04/14,12:42:16)
        */
        String getTime();

        /**!
         * @brief Reset the SIM7600 module, then waits for around 35 seconds to let SIM reinitialize
         */
        void resetModule();

        /**!
         * @brief Disconnect from AWS and release MQTT client
         */
        void disconnectAWS();

        /**!
         * @brief Handles incoming message from AWS, make sure to subscribe to topic beforehand. This example uses my ESP-Now custom library, feel free to change this function 
         * @param command is the command to send to another ESP-Now node (eg. "PUMPON" to turn on pump)
         * @param slaveName is the other ESP-Now SSID name to send to. If slaveName is "All", then will send to all connected ESP-Now peers
         * @param func1 is self-defined function
         */
        void checkResponseAWS(String check, String command1, String command2, String slaveName, void (&func)(String,String));

        /* Below are some helper functions  */

        /**!
         * @brief Prints the serial response from SIM7600 to another serial port
        */
        void printSerial();

        /**!
         * @brief Read serial response from SIM7600
         * @return the message received
         */
        String readSerial();

        /**!
         * @brief Another methodf for reading response from SIM7600 function 
        */
        String getResponse();

};


#endif