#include "SFDFSensor.h"


sensorNodes::sensorNodes(ModbusMaster* node_ec,ModbusMaster* node_ph,ModbusMaster* node_do)
:node_ec(node_ec), node_ph(node_ph), node_do(node_do){}

double sensorNodes::readDO()
{
    double value_do;
    uint16_t result = node_do->readHoldingRegisters (0x30, 0x01);
    if (result == node_do->ku8MBSuccess)
    {
        value_do = double(node_do->getResponseBuffer(0))/1000;
        // Serial.println (value_do);
    }

    return value_do;
}

double sensorNodes::readEC()
{
    double value_ec;
    uint16_t result = node_ec->readHoldingRegisters (0x00, 0x01);
    if (result == node_ec->ku8MBSuccess)
    {
        value_ec = node_ec->getResponseBuffer(0);
        // Serial.println (value_ec);
    }

    return value_ec;
}

double sensorNodes::readPh()
{
    double value_ph;
    uint16_t result = node_ph->readHoldingRegisters (0x09, 0x01);
    if (result == node_ph->ku8MBSuccess)
    {
        value_ph = double(node_ph->getResponseBuffer(0))/100;
        // Serial.println (value_ph);
    }

    return value_ph;
}

double sensorNodes::readTemperature()
{
    double value_temp;
    uint16_t result = node_do->readHoldingRegisters (0x2B, 0x01);
    if (result == node_do->ku8MBSuccess)
    {
        value_temp = double(node_do->getResponseBuffer(0))/100;
        // Serial.println (value_temp);
    }

    return value_temp;
}

double sensorNodes::readValue(ModbusMaster* node, uint16_t u16ReadAddress)
{
    double value = 0;
    uint16_t result = node->readHoldingRegisters(u16ReadAddress,1);
    if (result == node->ku8MBSuccess)
    {
        value = double(node->getResponseBuffer(0))/100;
    }
    return value;
}