/*
   DS18B20 temperature sensor routines
*/
#include <OneWire.h>
#include <DallasTemperature.h>
//////////////TEMPERATURE SENSOR CONTROL//////////////////////////

#define ONE_WIRE_BUS 3

union {
  DeviceAddress insideThermometer;
  uint64_t devBuf;
} thermometer;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// DeviceAddress insideThermometer;

int setupDS18B20(int entry) {
  sensors.begin();
  sensors.setResolution(12);
  sensors.getAddress(thermometer.insideThermometer, 0);
}

int pubDS18B20 (int entry) {

  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempC(thermometer.insideThermometer);

  sprintf(mqttData, " { \"Thermometer\": \"%08X\", \"Temperature\" : \"%.2f\"}",
          thermometer.devBuf, tempC);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
