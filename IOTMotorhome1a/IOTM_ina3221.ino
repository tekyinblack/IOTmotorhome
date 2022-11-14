/*

*/
// INA3221 sensor
#include <SDL_Arduino_INA3221.h>
SDL_Arduino_INA3221 ina3221;


int setupINA3221(int entry) {
  // Init INA3221
  setupWire();
  ina3221.begin();
}

int pubINA3221(int entry) {
  int sensor = mqttContext[entry].mqttParm1;
  if (sensor < 1 | sensor > 3) return 1;


  float busvoltage = ina3221.getBusVoltage_V(sensor);
  float current_mA = -ina3221.getCurrent_mA(sensor);
  float shuntvoltage = ina3221.getShuntVoltage_mV(sensor);

  sprintf(mqttData, "{\"sensor\": \"%d\", \"voltage\" : \"%.2f\",  \"current_mA\" : \"%.4f\"}",
          sensor,
          current_mA, 
          busvoltage);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
