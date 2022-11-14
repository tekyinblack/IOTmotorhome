/*
  INA219 sensor
  Thanks to Adafruit for supplying the librray I used and other libraries are available
*/

#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

int setupINA219(int entry) {
  // Init INA219
  setupWire();
  ina219.begin();

  return 0;
}

int pubINA219(int entry) {
  float current_mA = ina219.getCurrent_mA();
  float shuntvoltage = ina219.getShuntVoltage_mV();
  float busvoltage = ina219.getBusVoltage_V();
  float voltage_V = busvoltage - shuntvoltage;

  sprintf(mqttData, "{\"current\": \"%.4f\", \"voltage\" : \"%.2f\"}",
          current_mA, voltage_V);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
