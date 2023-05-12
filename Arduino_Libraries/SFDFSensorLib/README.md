# SFDF Sensor Library
This is a library for reading some water sensors data connected through Modbus with RS485 to ESP32. Used three sensors which are pH sensor, dissolved oxygen sensor, and EC sensor. Mainly utilizes ModbusMaster library. The sensor documentation are [here](./sensor_info/) (all in Chinese) and we changed the sensor addresses manually with their software.

## Libraries needed
Need the ModbusMaster Arduino library.

## Usage
Add this folder to your Arduino library. To see how to add this custom library [check this guide](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries). Check the example sketch in the examples folder.

These four functions are preset to the sensors we worked on with addresses already preset:
``` C++
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
```
You can change the addresses in the library if needed or use the function below.


For a bit more general function of reading from a Modbus node with passed node and address use function below:

``` C++
double readValue(ModbusMaster* node, uint16_t u16ReadAddress);

// example of usage:
double temperature_ex = nodes.readValue(&node_do,0x2B);
```