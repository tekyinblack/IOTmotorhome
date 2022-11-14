/*
 * ESPMotorhome is a firmware intended to support an IoT implementation for motorhomes, caravans etc
 * The main core of this is a timed loop which executes program functions as identified in an array 
 * in a regular manner. This is currently on a simple timer repetitive basis not on a wallclock basis
 * ie read data every 60 seconds etc. or in the case of a switch, this could be 0 seconds and hence continuously polled
 * 
 * The code outputs mqtt messages to topics held in the array but to only one broker for the purposes of data collection
 * and alerts, and will subscribe to mqtt topics specified in the array. If a display is implemented, this can be updated 
 * either on an individually timed basis or upon the collection of data, depending upon the array entry
 * 
 * This is an amalgamation of code from many sources listed below and in their invididual source code instances
*/
// Loading the ESP8266WiFi library and the PubSubClient library
#include <ESP8266WiFi.h>
//#include <Ethernet.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_INA219.h>
//#include <SDL_Arduino_INA3221.h>
//#include <Adafruit_BMP280.h>
//#include <PCF8574.h>
//#include <DHT.h>
//#include <ModbusMaster.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>


// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "SKYBC4E3";
const char* password = "RYTDXUFF";
// const char* ssid = "Lemmie_Net";
// const char* password = "ElizabethAnneOwen";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.1.219";
const char* mqtt_client = "ESPOther";

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);
IPAddress server(192, 168, 1, 219);
IPAddress displayAddress;

char logMessage[200];  // log messages not expected bigger than this


#define SOFTWARE 1
#define SWITCH 2
#define DHT11 3
#define BMP280 4
#define INA219 5
#define INA3221 6
#define DS18B20 7
#define ADS1115 8
#define MCP4725 9
#define RS485 10
#define PCF8591 11

int displayDummy (int ) {   // dummy display subroutune
}
int pubDummy (int ) {   // dummy publication subroutune
}
int setupDummy (int ) { // dummy setup routine
}
int subDummy (int , char* , byte* , unsigned int  ) {   // dummy subscription routine
}

struct mqttItem {
  int mqttID;                      // ID number for identification of entry, should be same as index but not essential
  int mqttElig;                    // is this entry active and eligible for processing
  int mqttType;                    // a type descriptor should it be useful for general subroutines
  char mqttPubTopic[20];           // publication topic name for entry
  char mqttSubTopic[20];           // subscrption topic name for entry
  int mqttPub;                     // flag to indicate this entry publishes data
  int mqttSub;                     // flag to indicate this entry subscribes to data
  int mqttDsp;                     // flag to indicate this entry subscribes to data
  int (*mqttPubFunction)(int) ;    // address of publication  subroutine for entry
  int (*mqttSubFunction) (int , char* , byte* , unsigned int );    // address of subscription subroutine for entry
  int (*mqttSetupFunction)(int) ;  // address of setup subroutine for entry
  int (*mqttDisplayFunction)(int) ;  // address of display subroutine for entry
  long mqttTimer;                  // timer value for entry
  long mqttDelay;                  // delay value for entry
  uint8_t mqttParm1;                // 1st parameter for item , a small positive integer, useful for most options
  uint8_t mqttParm2;                // 2nd paramater for item , a small positive integer, useful for most options
  uint8_t mqttParm3;                // 3rd parameter for item
  uint8_t mqttParm4;                // 4th paramater for item

};

struct mqttItem  mqttContext[] =  {
  //   ID elig, type            PubTopic        SubTopic          Pub  Sub  Dsp PublishFunction    SubscribeFunction  SetupFunction   DisplayFunction Timer  Delay  Parm1 Parm2 Parm3 Parm4
      {0,  1,   SOFTWARE,       "SoftwareOut",  "SoftwareIn",     1,   1,   0,  &pubSoftware,      subSoftware,       setupSoftware,  displayDummy,   0,     10000, 0,    0,    0,    0       },  //  software only handler
      {1,  0,   SWITCH,         "topic",        "subTopic",       0,   0,   0,  pubPCF8574,        subPCF8574,        setupPCF8574,   displayDummy,   0,     10000, 0,    0,    0,    0       },  //  relay/switch handler
      {2,  0,   DHT11,          "DHT11",        "subTopic",       1,   0,   0,  pubDHT11,          subDummy,          setupDHT11,     displayDummy,   0,     10000, 0,    0,    0,    0       },  //  DHT11 data source
      {3,  0,   BMP280,         "BMP280",       "subTopic",       1,   0,   0,  pubBMP280,         subDummy,          setupBMP280,    displayDummy,   0,     10000, 0,    0,    0,    0       },  //  BMP280 data source
      {4,  0,   INA219,         "topic",        "subTopic",       0,   0,   0,  pubINA219,         subDummy,          setupINA219,    displayDummy,   0,     10000, 0,    0,    0,    0       },  //  INA219 data source
      {5,  0,   INA3221,        "topic",        "subTopic",       0,   0,   0,  pubINA3221,        subDummy,          setupINA3221,   displayDummy,   0,     10000, 0,    0,    0,    0       },  //  INA3221 data source
      {6,  0,   DS18B20,        "ds18b20",      "subTopic",       1,   0,   0,  pubDS18B20,        subDummy,          setupDS18B20,   displayDummy,   0,     10000, 0,    0,    0,    0       },  //  DS18B20 data source
      {7,  1,   ADS1115,        "ads1115",      "subTopic",       1,   0,   0,  pubADS1115,        subDummy,          setupADS1115,   displayDummy,   0,     10000, 0,    0,    0,    0       },  //  ADS1115 data source  
      {8,  0,   MCP4725,        "mcp4725",      "subTopic",       1,   0,   0,  pubDummy,          subDummy,          setupDummy,     displayDummy,   0,     10000, 0,    0,    0,    0       },  //  MCP4725 DAC voltage data source     
      {9,  0,   PCF8591,        "pcf8591",      "subPcf8591",     1,   0,   0,  pubPCF8591,        subPCF8591,        setupPCF8591,   displayDummy,   0,     10000, 0,    0,    0,    0       },  //  MCP4725 DAC voltage data source         
     {10,  0,   RS485,          "solarLive",    "subTopic",       1,   0,   0,  pubEpsolarLive,    subDummy,          setupEpsolar,   displayDummy,   0,     10000, 0,    0,    0,    0       }   //  epsolar data source
};
int itemCount = (sizeof(mqttContext) / sizeof(mqttContext[0])); // variable for number of items in mqttContext array

// Initializes the espClient
// STATEMENTS FOR WiFi
WiFiClient espClient;
PubSubClient client(espClient);

// STATEMENTS FOR Ethernet
//EthernetClient ethClient;
//PubSubClient client(ethClient);

// Timers auxiliary variables
long newMeasure = 0;

char mqttData[1024];
int ota = 0;

/*
   Wire setup routine. Called from setup routines that need the I2C wire connection.
   It ensure that wire.begin executes only once
*/
int wireSetup = 0;
void setupWire(void) {
  if (wireSetup) return;
  Wire.begin(2, 0);
  wireSetup = 1;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Started");
  Serial.println(sizeof(mqttContext[0]));
  //  Wire.begin(2,0);
  delay(1000);

  for (int i = 0; i < itemCount; i++) {
    if (mqttContext[i].mqttElig == 1) {
      mqttContext[i].mqttSetupFunction(i);
    }
  }
  setup_wifi();   // <-------- WiFI
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //   setupEth();   // <---------------Ethernet
  if (!client.connected()) {
    reconnect();
  }
  ArduinoOTA.begin();
  sprintf(logMessage, "Sketch: %s  Uploaded: %s %s", __FILE__, __DATE__, __TIME__);
  pubLog();
}



void loop() {
/*
 * Each loop starts with an update handler statement. A loop is included for first startup to 
 * aid in recovering a crashing node by being able to update it with stable code
 */
  ArduinoOTA.handle();
  while (ota < 20) {    // loop for 5 seconds to give a chance to update after reboot if code crashing
                        // obviously increase the length of this loop if its helpful
    delay(250);
    ArduinoOTA.handle();
    ota++;
  } ;

  if (!client.connected()) {   // reconnect mqtt if connection lost
    reconnect();
  }
  client.loop();               // process all subscriptions
/*
 * The following loops through the array processing all eligible items
 */
  for (int i = 0; i < itemCount; i++) {
    if (mqttContext[i].mqttElig == 1) {
      if (mqttContext[i].mqttPub == 1 ) {
        newMeasure = millis();
        if (newMeasure >= mqttContext[i].mqttTimer) {
          mqttContext[i].mqttTimer = newMeasure + mqttContext[i].mqttDelay;

          int status = mqttContext[i].mqttPubFunction(i);
          if (mqttContext[i].mqttDsp == 1 ) {
            int status = mqttContext[i].mqttDisplayFunction(i);    // where an item has an associated display option, process it immediatley afterwards
          }
        }
      }

      if (mqttContext[i].mqttDsp == 1 ) {
        newMeasure = millis();
        if (newMeasure >= mqttContext[i].mqttTimer) {
          mqttContext[i].mqttTimer = newMeasure + mqttContext[i].mqttDelay;

          int status = mqttContext[i].mqttDisplayFunction(i);     // process a display item
        }
      }
    }
  }
}
