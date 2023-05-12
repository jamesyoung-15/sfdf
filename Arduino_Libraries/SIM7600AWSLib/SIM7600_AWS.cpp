#include "SIM7600_AWS.h"

SIM7600AWS::SIM7600AWS(Stream *simPort, Stream *printPort): sim7600Port(simPort), printSerialPort(printPort){};

void SIM7600AWS::testSim(String command)
{
    sim7600Port->println(command);
    printSerial();
    delay(50);
}

void SIM7600AWS::configureSSL(String cacert, String clientcert, String clientkey)
{
    // remember for double quotes " use escape sequence \ when sending string like this \"

    // Set the SSL version of the first SSL context
    sim7600Port->println("AT+CSSLCFG=\"sslversion\",0,4");
    delay(100);

    // Set the authentication mode to verify server and client
    sim7600Port->println("AT+CSSLCFG=\"authmode\",0,2");
    delay(100);
    // Set the server root CA of the first SSL contex
    sim7600Port->println("AT+CSSLCFG=\"cacert\",0,\""+ cacert +".pem\"");
    delay(100);

    // Set the client certificate of the first SSL context
    sim7600Port->println("AT+CSSLCFG=\"clientcert\",0,\""+ clientcert +".pem\"");
    delay(100);

    // Set the client key of the first SSL context
    sim7600Port->println("AT+CSSLCFG=\"clientkey\",0,\"" + clientkey + ".pem\"");
    delay(100);
    
    // open network
    sim7600Port->println("AT+NETOPEN");

}

void SIM7600AWS::connectAWS(String clientName, String awsEndpoint)
{
    // start MQTT service
    sim7600Port->println("AT+CMQTTSTART");
    printSerial();
    delay(4000);

    // Acquire one client which will connect to a SSL/TLS MQTT server
    sim7600Port->println("AT+CMQTTACCQ=0,\""+ clientName + "\",1");
    printSerial();
    delay(4000);

    // Set the first SSL context to be used in the SSL connection
    sim7600Port->println("AT+CMQTTSSLCFG=0,0");
    printSerial();
    delay(4000);

    // Connect to a MQTT server, in this case aws
    // printSerialPort->println("AT+CMQTT Connect");
    // Serial2.println("AT+CMQTTCONNECT=0,\"tcp://a5sswhj5ru4gy-ats.iot.us-east-2.amazonaws.com:8883\",60,1");
    sim7600Port->println("AT+CMQTTCONNECT=0,\"tcp://"+ awsEndpoint + ":8883\",60,1");
    printSerial();
    delay(5000);
}

void SIM7600AWS::subscribeTopic(String topic)
{
    // subscribe to topic
    sim7600Port->println("AT+CMQTTSUB=0,"+String(topic.length())+",1");
    delay(50);
    sim7600Port->println(topic);
    delay(100);
    printSerial();
}

void SIM7600AWS::sendDataAWS(String topic, String message)
{
      // Set the topic for the PUBLISH message
    // printSerialPort->println("AT+CMQTTTOPIC=0,"+String(topic.length()));
    sim7600Port->println("AT+CMQTTTOPIC=0,"+String(topic.length()));
    delay(20);
    sim7600Port->println(topic); // inputs topic name
    printSerial();
    delay(300);

    // Set the payload for the PUBLISH message
    sim7600Port->println("AT+CMQTTPAYLOAD=0," + String(message.length())); 
    delay(20);
    sim7600Port->println(message); // input message to be sent
    printSerial();
    delay(300);

    // Publish message
    sim7600Port->println("AT+CMQTTPUB=0,1,60");
    printSerial();  
}

void SIM7600AWS::sendSensorData(String topic, double ph, double ec, double do_data, double temperature)
{
    is_sending_aws = 1;
    // Set the topic for the PUBLISH message
    printSerialPort->println("AT+CMQTTTOPIC=0,"+String(topic.length()));
    sim7600Port->println("AT+CMQTTTOPIC=0,"+String(topic.length()));
    delay(20);
    sim7600Port->println(topic); // inputs topic name
    printSerial();
    delay(300);

    // create json document
    StaticJsonDocument<200> doc;

    // create nested json object data
    JsonObject data = doc.createNestedObject("data");

    // add sensor data to object data
    data["pH"] = ph;
    delay(50);
    data["EC"] = ec;
    delay(50);
    data["DO"] = do_data;
    delay(50);
    data["Temp"] = temperature;
    delay(50);
    data["DateTime"] = getTime();
    
    // convert json to string
    String message;
    serializeJson(doc, message);
    printSerialPort->println("\n"+message);

    // is_sending_aws = 1;
    // Set the payload for the PUBLISH message
    sim7600Port->println("AT+CMQTTPAYLOAD=0," + String(message.length())); 
    delay(20);
    sim7600Port->println(message); // input json message to be sent
    // is_sending_aws = 0;
    printSerial();
    delay(300);

    // Publish message
    sim7600Port->println("AT+CMQTTPUB=0,1,60");
    printSerial();  
    is_sending_aws = 0;
}

String SIM7600AWS::getTime()
{
    // Get real time clock management of SIM module, full format is “yy/MM/dd,hh:mm:ss±zz”, eg.(+CCLK: “08/11/28,12:30:35+32”)
    sim7600Port->println("AT+CCLK?");
    String temp = getResponse();
    // Cconcatenate response to format of: YY/MM/DD, HH:MM:SS (eg. 23/04/14,12:42:16)
    String time = temp.substring(19,36);
    printSerialPort->println(time);
    return time;
}

void SIM7600AWS::printSerial()
{
    while (sim7600Port->available()) 
    {
        printSerialPort->println(sim7600Port->readString());
    }
}

String SIM7600AWS::readSerial()
{
    String response;
    while(sim7600Port->available())
    {
        response = sim7600Port->readString();
    }    
    return response;
}

String SIM7600AWS::getResponse()
{
    String response;

    response = sim7600Port->readString();

    response.trim(); // remove white space
    return response;
}

void SIM7600AWS::resetModule()
{
    sim7600Port->println("AT+CRESET");
    printSerial();
    delay(35000);          
}

void SIM7600AWS::disconnectAWS()
{
    // disconnect from server
  sim7600Port->println("AT+CMQTTDISC=0,120");
  printSerial();
  delay(50);

  // release client
  sim7600Port->println("AT+CMQTTREL=0");
  printSerial();
  delay(50);

  // stop mqtt service
  sim7600Port->println("AT+CMQTTSTOP");
  printSerial();
  delay(50);
}

void SIM7600AWS::checkResponseAWS(String check, String command1, String command2, String slaveName, void (&func)(String,String))
{
    // if we receive topic from AWS, Serial2 will get message
    if(sim7600Port->available()>0)
    {
        String received = sim7600Port->readString();
        // AWS payload message will contain response and either ok or error
        if(received.indexOf(check)>0)
        {
            printSerial();
            printSerialPort->println("Received response");

            // check if the sent message contains some String command, then do some function
            if(received.indexOf(command1)>0)
            {
                func(command1, slaveName);
            }
            else if(received.indexOf(command2)>0)
            {
                func(command2, slaveName);
            }
            
        }
    }
}



