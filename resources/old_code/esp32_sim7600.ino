#include <ModbusMaster.h>
#include <ArduinoJson.h>

// Serial 1, pin 16 (U2RX) and 17 (U2TX)
#define RXD2 16
#define TXD2 17

// Serial 2, pin 18 (U1RX) and 19 (U1TX)
#define RXD1 18
#define TXD1 19

// ModMaster Nodes, for getting data with modbus
ModbusMaster node_ec;
ModbusMaster node_ph;
ModbusMaster node_do;

// LED variables
const uint8_t AIN1 = 33;
const uint8_t AIN2 = 32;
const uint8_t BIN1 = 25;
const uint8_t BIN2 = 26;

int led_stat = 0; // current led signal
long led_prev_mills = 0;

// millis variable for sending data every x seconds without using delay
unsigned long pervious_sent_millis = 0; 
const long send_interval = 30000; // interval to send data to AWS, in milliseconds


// AWS variables, change these for each client and topic you desire
String publish_topic_name = "sfdf/client01/sensor_data";
String client_name = "sfdf_client01";
String subscribe_topic_name = "sfdf/client01/response";


void setup() {
  // put your setup code here, to run once:

  // Start hardware serial. 
  Serial.begin(115200);
  while (!Serial) {; } // wait for serial port to connect
  delay(1000);

  // Start Serial2
  Serial2.begin (115200, SERIAL_8N1, RXD2, TXD2);
  while(!Serial2){;}
  delay(2000);

  // Start serial 1
  Serial1.begin(9600,SERIAL_8N1,RXD1,TXD1);
  delay(1000);

  // Start sensor modmaster nodes
  node_ph.begin (1, Serial1);
  node_ec.begin (2, Serial1);
  node_do.begin (3, Serial1);

  // Example: Write AT command and get response, should return OK in serial
  Serial2.println("AT");
  print_serial();
  delay(500);

  // Disconnect from AWS if module still connected
  disconnect_aws();

  // Reset module, not necessary but may be handy at times
  // reset_module();



  // Configure SSL
  configure_ssl();
  delay(2000);

  // Connect AWS
  connect_aws(client_name);
  print_serial();
  delay(2000);

  // Subscribe to topic
  subscribe_topic(subscribe_topic_name);

}

void loop() {
  // send data to AWS every 30 seconds
  unsigned long current_millis = millis();
  if(current_millis - pervious_sent_millis >= send_interval)
  {
    pervious_sent_millis = current_millis;
    send_data(publish_topic_name);
  }
  // delay(30000); 

  // error detection of sensor readings by AWS payload
  // if we receive topic from AWS, Serial2 will get message
  if(Serial2.available()>0)
  {
    String received = Serial2.readString();
    // AWS payload message will contain response and either ok or error
    if(received.indexOf("response")>0)
    {
      Serial.println("Received response");
      
      if(received.indexOf("ERROR")>0)
      {
        Serial.println("There is an error!");
        led_changestat('r');

      }
      else if(received.indexOf("OK")>0)
      {
        Serial.println("No error");
        led_changestat('o');
      }      
    }
  }

}

// read serial response and return the message received
String read_serial()
{
  String response;
  while(Serial2.available())
  {
    response = Serial2.readString();
  }    
  return response;
}

// Print Sim7600 from software serial response to Serial port
void print_serial()
{
  while (Serial2.available()) 
  {
    Serial.println(Serial2.readString());
  }
}



// Return sim7600 response as a string, for cases like when wanting to compare response to expected response
String get_response()
{
  String response;

  response = Serial2.readString();

  response.trim(); // remove white space
  return response;
}

// Configure the SSL, certificates need to be downloaded into the SIM7600 module beforehand with the right corresonding file names set. More info check documentation and examples
void configure_ssl()
{
  // remember for double quotes " use escape sequence \ when sending string like this \"

  // Set the SSL version of the first SSL context
  Serial2.println("AT+CSSLCFG=\"sslversion\",0,4");
  delay(100);

  // Set the authentication mode to verify server and client
  Serial2.println("AT+CSSLCFG=\"authmode\",0,2");
  delay(100);
  // Set the server root CA of the first SSL contex
  Serial2.println("AT+CSSLCFG=\"cacert\",0,\"cacert.pem\"");
  delay(100);

  // Set the client certificate of the first SSL context
  Serial2.println("AT+CSSLCFG=\"clientcert\",0,\"clientcert.pem\"");
  delay(100);

  // Set the client key of the first SSL context
  Serial2.println("AT+CSSLCFG=\"clientkey\",0,\"clientkey.pem\"");
  delay(100);
  
  // open network
  Serial2.println("AT+NETOPEN");

}

// Connect to AWS, parameter is the client name
void connect_aws(String client_name)
{
  // start MQTT service
  Serial2.println("AT+CMQTTSTART");
  print_serial();
  delay(4000);

  // Acquire one client which will connect to a SSL/TLS MQTT server
  Serial2.println("AT+CMQTTACCQ=0,\""+ client_name + "\",1");
  print_serial();
  delay(4000);

  // Set the first SSL context to be used in the SSL connection
  Serial2.println("AT+CMQTTSSLCFG=0,0");
  print_serial();
  delay(4000);

  // Connect to a MQTT server, in this case aws
  // Serial.println("AT+CMQTT Connect");
  Serial2.println("AT+CMQTTCONNECT=0,\"tcp://a5sswhj5ru4gy-ats.iot.us-east-2.amazonaws.com:8883\",60,1");
  print_serial();
  delay(5000);
}

// Create MQTT topic, create json with relevant sensor data, publish payload to AWS
// Parameter is the topic name for the published data that AWS will subscribe to
// Json will look something like this:
// {
//   "data": {
//     "pH": 8.96,
//     "EC": 3.67,
//     "DO": 2.04,
//     "Temp": 24.2,
//     "DateTime": "23/04/14,12:18:55"
//   }
// }
void send_data(String topic)
{
  // Set the topic for the PUBLISH message
  Serial.println("AT+CMQTTTOPIC=0,"+String(topic.length()));
  Serial2.println("AT+CMQTTTOPIC=0,"+String(topic.length()));
  delay(20);
  Serial2.println(topic); // inputs topic name
  print_serial();
  delay(300);

  // create json document
   StaticJsonDocument<200> doc;

   // create nested json object data
  JsonObject data = doc.createNestedObject("data");

  // add sensor data to object data
  data["pH"] = read_ph();
  delay(50);
  data["EC"] = read_ec();
  delay(50);
  data["DO"] = read_do();
  delay(50);
  data["Temp"] = read_temp();
  delay(50);
  data["DateTime"] = get_time();
  
  // convert json to string
  String message;
  serializeJson(doc, message);
  Serial.println("\n"+message);

  // Set the payload for the PUBLISH message
  Serial2.println("AT+CMQTTPAYLOAD=0," + String(message.length())); 
  delay(20);
  Serial2.println(message); // input json message to be sent
  print_serial();
  delay(300);

  // Publish message
  Serial2.println("AT+CMQTTPUB=0,1,60");
  print_serial();  
}


// Subscribe to topic from AWS to receive data if needed
void subscribe_topic(String topic)
{
  // subscribe to topic
  Serial2.println("AT+CMQTTSUB=0,"+String(topic.length())+",1");
  delay(50);
  Serial2.println(topic);
  delay(100);
  print_serial();
    
}

// disconnects from all MQTT server
void disconnect_aws()
{
  // disconnect from server
  Serial2.println("AT+CMQTTDISC=0,120");
  print_serial();
  delay(50);

  // release client
  Serial2.println("AT+CMQTTREL=0");
  print_serial();
  delay(50);

  // stop mqtt service
  Serial2.println("AT+CMQTTSTOP");
  print_serial();
  delay(50);
}

// Returns string of current time in UTC+8 in format of “YY/MM/DD,HH:MM:SS” (eg. 23/04/14,12:42:16)
String get_time()
{
  // Get real time clock management of SIM module, full format is “yy/MM/dd,hh:mm:ss±zz”, eg.(+CCLK: “08/11/28,12:30:35+32”)
  Serial2.println("AT+CCLK?");
  String temp = get_response();
  // Cconcatenate response to format of: YY/MM/DD, HH:MM:SS (eg. 23/04/14,12:42:16)
  String time = temp.substring(19,36);
  Serial.println(time);
  return time;
}

void reset_module()
{
  Serial2.println("AT+CRESET");
  print_serial();
  delay(35000);
}


// read pH value from pH sensor; Address: 0x01; return as double
double read_ph ()
{
    double value_ph;
    uint16_t result = node_ph.readHoldingRegisters (0x09, 0x01);
    if (result == node_ph.ku8MBSuccess)
    {
        value_ph = double(node_ph.getResponseBuffer(0))/100;
        // Serial.println (value_ph);
    }

    return value_ph;
}

// read electrical conductivity from EC sensor; Address: 0x02; return as double in unit uS/cm
double read_ec ()
{
    double value_ec;
    uint16_t result = node_ec.readHoldingRegisters (0x00, 0x01);
    if (result == node_ec.ku8MBSuccess)
    {
        value_ec = node_ec.getResponseBuffer(0);
        // Serial.println (value_ec);
    }

    return value_ec;
}

// read Disolved Oxygen value from DO sensor; Address: 0x03; return as double in unit mg/L
double read_do ()
{
    double value_do;
    uint16_t result = node_do.readHoldingRegisters (0x30, 0x01);
    if (result == node_do.ku8MBSuccess)
    {
        value_do = double(node_do.getResponseBuffer(0))/1000;
        // Serial.println (value_do);
    }

    return value_do;
}

// read temperature from DO sensor; Address: 0x03; return double in °C
double read_temp ()
{
    double value_temp;
    uint16_t result = node_do.readHoldingRegisters (0x2B, 0x01);
    if (result == node_do.ku8MBSuccess)
    {
        value_temp = double(node_do.getResponseBuffer(0))/100;
        // Serial.println (value_temp);
    }

    return value_temp;
}

// implementation of sensor data error detection without AWS
void detect_error()
{

}



