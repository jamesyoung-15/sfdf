#include <ModbusMaster.h>
#include <ArduinoJson.h>

// Serial 2, pin 16 (U2RX) and 17 (U2TX)
#define RXD2 16
#define TXD2 17

// Serial 1, pin 18 (U1RX) and 19 (U1TX)
#define RXD1 18
#define TXD1 19

// ModMaster Nodes
ModbusMaster node_ec;
ModbusMaster node_ph;
ModbusMaster node_do;

void setup() {
  // put your setup code here, to run once:

  // Start hardware serial. 
  Serial.begin(115200);
  while (!Serial) {; } // wait for serial port to connect
  delay(1000);

  // Start software Serial2. For testing where hardware serial is usb to computer and software serial is ESP32 to SIM module
  Serial2.begin (115200, SERIAL_8N1, RXD2, TXD2);
  while(!Serial2){;}
  delay(2000);

  // Start serial 1. For testing it is used for sensors
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

  // Reset module if needed
  reset_module();



  // Configure SSL
  configure_ssl();
  delay(2000);

  // Connect AWS
  connect_aws();
  print_serial();
  delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  send_data();
  delay(30000);

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

  response.trim();
  return response;
}

// Configure the SSL
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

// Connect to AWS
void connect_aws()
{
  // start MQTT service
  Serial2.println("AT+CMQTTSTART");
  print_serial();
  delay(4000);

  // Acquire one client which will connect to a SSL/TLS MQTT server
  Serial2.println("AT+CMQTTACCQ=0,\"sim_client01\",1");
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

// Create MQTT topic, create json with relevant sensor data, send payload to AWS
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
void send_data()
{
  // Set the topic for the PUBLISH message
  String topic = "sfdf/client01";
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


/*  changes LED based on parameter passed
    parameter should be 'o' for off, 'r' for red, 'y' for yellow, 'g' for green */
void led_changestat (char led_color)
{
    // Set switch case for LED status
    // led_stat : 0 = Off; 1 = Red; 2 = Yellow; 3 = Green
    switch (led_color)
    {
        case 'o':
        digitalWrite (AIN1, !HIGH);
        digitalWrite (AIN2, !HIGH);
        digitalWrite (BIN1, !HIGH);
        digitalWrite (BIN2, !HIGH);
        led_stat = 0;
        break;

        case 'r':
        digitalWrite (AIN1, HIGH);
        digitalWrite (AIN2, !HIGH);
        digitalWrite (BIN1, !HIGH);
        digitalWrite (BIN2, !HIGH);
        led_stat = 1;
        break;

        case 'y':
        digitalWrite (AIN1, !HIGH);
        digitalWrite (AIN2, HIGH);
        digitalWrite (BIN1, !HIGH);
        digitalWrite (BIN2, !HIGH);
        led_stat = 2;
        break;

        case 'g':
        digitalWrite (AIN1, !HIGH);
        digitalWrite (AIN2, !HIGH);
        digitalWrite (BIN1, HIGH);
        digitalWrite (BIN2, !HIGH);
        led_stat = 3;
        break;     
    }
}

int led_blink (char led_color, long interval_time)
{
    unsigned long cur_millis = millis();

    if (cur_millis - led_prev_mills > interval_time)
    {
       led_prev_mills = cur_millis;

       if (led_stat == 0 || led_stat == 99)
       {
            led_changestat (led_color);
       } 
       else
       {
           led_changestat ('o');
       }
    }
}