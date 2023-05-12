#include <ModbusMaster.h>

#define RXD2 16
#define TXD2 17

ModbusMaster node_ec;
ModbusMaster node_ph;
ModbusMaster node_do;

void setup ()
{
    Serial.begin (9600);
    Serial2.begin (9600, SERIAL_8N1, RXD2, TXD2);

    node_ph.begin (1, Serial2);
    node_ec.begin (2, Serial2);
    node_do.begin (3, Serial2);
}

void loop ()
{
    double value_ec = read_ec ();
    delay (50);
    double value_ph = read_ph ();
    delay (50);
    double value_do = read_do ();
    delay (50);
    double value_temp = read_temp ();

    Serial.println ("EC : " + String(value_ec,1) + " uS/cm");
    Serial.println("pH : " + String (value_ph,1));
    Serial.println("DO : " + String(value_do,1) + " mg/L");
    Serial.println("Temp : " + String(value_temp, 1) + " deg C");
    Serial.println ("-------------------------------");

    delay (1000);
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