#include "SFDFSensor.h"

// Serial 1, pin 18 (U1RX) and 19 (U1TX)
#define RXD1 18
#define TXD1 19


// ModMaster Nodes, for getting data with modbus
ModbusMaster node_ec;
ModbusMaster node_ph;
ModbusMaster node_do;

sensorNodes nodes(&node_ec,&node_ph,&node_do);

void setup() {
  Serial.begin(115200);
  // Start serial 1
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  delay(1000);

  // Start sensor modmaster nodes
  node_ph.begin(1, Serial1);
  node_ec.begin(2, Serial1);
  node_do.begin(3, Serial1);

}

void loop() {

  // Example of usage
  double temperature = nodes.readTemperature();
  Serial.printf("%.2f\n", temperature);

  // Example of entering manually, pass the node you want to read from and the address
  double temperature_ex = nodes.readValue(&node_do,0x2B);
  Serial.printf("%.2f\n",temperature_ex);
  delay(5000);

}
