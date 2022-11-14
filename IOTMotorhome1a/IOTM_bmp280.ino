/*

*/
// BMP280 sensor
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp; // I2C

int setupBMP280(int entry) {
  // Init BMP280
  setupWire();
  bmp.begin();

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  return 0;
}

int pubBME280(int entry) {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();

  sprintf(mqttData, " { \"temperature\": \"%.2f\", \"pressure\" : \"%.2f\"}",
          temperature, pressure);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
