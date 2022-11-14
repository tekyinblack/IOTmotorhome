
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>



Adafruit_BME280 bme; // I2C


int setupBME280(int entry) {
  // Init BME280
  setupWire();
  bme.begin();

  /* Default settings from datasheet. */
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BME280::SAMPLING_X16,     /* Temp. oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Humidity oversampling */
                  Adafruit_BME280::FILTER_X16,      /* Filtering. */
                  Adafruit_BME280::STANDBY_MS_500); /* Standby time. */
  return 0;
}

int pubBMP280(int entry) {
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure();
  float humidity = bme.readHumidity();

  sprintf(mqttData, " { \"temperature\": \"%.2f\", \"pressure\" : \"%.2f\", \"humidity\" : \"%.2f\"}",
          temperature, pressure, humidity);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
