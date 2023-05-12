#include <ArduinoModbus.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>


ModbusMaster node1;
ModbusMaster node2;
ModbusMaster node3;
void setup()
{
  // use Serial (port 0); initialize Modbus communication baud rate
  Serial.begin(9600);
  Serial3.begin(9600);
  node1.begin(1, Serial3);
  node2.begin(2, Serial3);
  node3.begin(3, Serial3);
}


void loop()
{
  readPh();
  delay(50);
  readEc();
  delay(50);
  readDo();
  delay(1000);
}

// read hex return from serial, used when trying to reverse find sent hex command from there software
void readSerial()
{
  while(Serial3.available())
  {
    uint8_t rxByte;
    rxByte = Serial3.read();
    Serial.print((rxByte >> 4) & 0x0F, HEX);
    Serial.print(rxByte & 0x0F, HEX);
    Serial.println(" ");
  }
}

// read ph value from sensor assuming id is device address is 0x2, expected normal is 8-9 ph
void readPh()
{
  // node.begin(2, Serial3);
  uint16_t result = node2.readHoldingRegisters(0x09, 0x01);
  if (result == node2.ku8MBSuccess)
  {
    double value_ph = double(node2.getResponseBuffer(0))/100;
    Serial.print(value_ph);
    Serial.println(" ph");
  }
  // return value_ph
}

// read ec value assuming device address is 0x2, expected is 
void readEc()
{
  // node.begin(1,Serial3);
  uint16_t result = node1.readHoldingRegisters(0x00, 0x01);
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("EC: ");
    Serial.print(node1.getResponseBuffer(0));
    Serial.println("uS/cm");
  }
  // return value_ph  
}

// read ec value assuming device address is 0x2, expected is less than 5
void readDo()
{
  // node.begin(3,Serial3);
  // get do value
  uint16_t result_do = node3.readHoldingRegisters(0x30, 0x01);
  if (result_do == node3.ku8MBSuccess)
  {
    double value_do = double(node3.getResponseBuffer(0))/1000;    
    // double temperature = double(node.getResponseBuffer(0))/1000;
    Serial.print("DO: ");
    Serial.print(value_do);
    Serial.println(" mg/L ");
  }
  uint16_t result_temp = node3.readHoldingRegisters(0x2B, 0x01);
  if (result_temp == node3.ku8MBSuccess)
  {
    double value_temp = double(node3.getResponseBuffer(0))/100;    
    // double temperature = double(node.getResponseBuffer(0))/1000;
    Serial.print("Temp: ");
    Serial.print(value_temp);
    Serial.println(" C ");
  }
  // return value_ph
}
