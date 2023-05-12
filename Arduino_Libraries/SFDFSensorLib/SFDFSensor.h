#ifndef SFDFSENSOR_H
#define SFDFSENSOR_H

#include <Arduino.h>
#include <ModbusMaster.h>

class sensorNodes
{
    private:
        ModbusMaster* node_ec;
        ModbusMaster *node_ph;
        ModbusMaster *node_do;
    public:
        /**!
         * @brief Constructor initialize the connection to each sensor.
         * @param node_ec: Modbus node for EC sensor
         * @param node_ph: Modbus node for ph sensor
         * @param node_do: Modbus node for dissolved oxygen sensor
        */
        sensorNodes(ModbusMaster* node_ec,ModbusMaster *node_ph,ModbusMaster* node_do);

        /**!
         * @brief Reads the EC value from the sensor register, unit in uS/cm
         * @return Returns EC value in uS/cm.
         */
        double readEC();

        /**!
         * @brief Reads the temperature from the sensor register, unit in celsius 
         * @return Temperature in celsius.
         */
        double readTemperature();


        /**!
         * @brief Reads the disolved oxygen value from the sensor register, unit in mg/L
         * @return Returns the disolved oxygen value in mg/L.
         */
        double readDO();

        /**!
         * @brief Reads the pH value of the sensor from the sensor register, unit in pH.
         * @return Returns pH value.
         */
        double readPh();

        /**!
         * @brief General function for reading Modbus sensor slave with readHoldingRegisters function from ModBusMaster, will only read one quantity of holding register.
         * @param ModbusMaster object that you wish to use/read from.
         * @param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
         * @return Value of sensor data
         */
        double readValue(ModbusMaster* node, uint16_t u16ReadAddress);
        
};



#endif