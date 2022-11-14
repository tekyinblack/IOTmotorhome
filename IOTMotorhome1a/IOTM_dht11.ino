/*

*/
#include <DHT.h>
// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// DHT Sensor
const int DHTPin = 3;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

int setupDHT11 (int entry) {
  dht.begin();
}

int pubDHT11 (int entry) {

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Computes temperature values in Celsius
  float hic = dht.computeHeatIndex(t, h, false);
  static char temperatureTemp[7];
  dtostrf(hic, 6, 2, temperatureTemp);


  static char humidityTemp[7];
  dtostrf(h, 6, 2, humidityTemp);

  sprintf(mqttData, " { \"temperature\": \"%.2f\", \"humidity\" : \"%.2f\"}",
          temperatureTemp, humidityTemp);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
