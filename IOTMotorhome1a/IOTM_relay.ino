/*
   Relay device routines
     //Hex command to send to serial for close relay
  byte relON[]  = {0xA0, 0x01, 0x01, 0xA2};

  //Hex command to send to serial for open relay
  byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1};

  //Hex command to send to serial for close relay
  byte rel2ON[]  = {0xA0, 0x02, 0x01, 0xA3};

  //Hex command to send to serial for open relay
  byte rel2OFF[] = {0xA0, 0x02, 0x00, 0xA2};
*/
union {
struct {
  uint8_t base;//  = 0xA0;
  uint8_t unit;
  uint8_t state;
  uint8_t parity;
} data;
  byte command[4];
} relay;

int pubRelay (int entry) {
  displayAddress =  WiFi.localIP();
  sprintf(mqttData, " { \"Softparm1\": \"%d.%d.%d.%d\", \"Softparm2\" : \"%d\"}", 
           displayAddress[0],displayAddress[1],displayAddress[2],displayAddress[3],millis());

  

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}

int setupRelay (int entry) {
  Serial.begin(9600);
}

int subRelay (int entry, char* topic, byte* payload, unsigned int length ) {

   uint8_t relayNo;
   uint8_t relayState;

   relay.data.unit = relayNo;
   relay.data.state = relayState;
   relay.data.parity = relay.data.base | relay.data.unit | relay.data.state;

   for (int i; i<4;i++) {
    Serial.write(relay.command[i]);
   }

    sprintf(logMessage, "relay %d activated", relayNo);
    pubLog();
  return 0;
}
