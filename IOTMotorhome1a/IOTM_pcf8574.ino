/*
   ESP Switch module
   uses PCF8574

   Default is to use pin as a digital input, bit map held it array parm1
   This isn't really easy to read but its easy to store!

*/
#include <PCF8574.h>
//PCF8574 pcf8574(0x38);

PCF8574 pcf8574(0x20);


int setupPCF8574(int entry) {

  uint8_t pinSet;

  setupWire();

  pinSet = mqttContext[entry].mqttParm1 & 0xFF;

  for (int i = 0; i < 8; i++) {
    if (bitRead(pinSet, i)) {
      pcf8574.pinMode(i, OUTPUT); // powercntlpin
    } else pcf8574.pinMode(i, INPUT);
  }
  pcf8574.begin();

  pinSet = pinSet & mqttContext[entry].mqttParm2;

  for (int i = 0; i < 8; i++) {
    if (bitRead(pinSet, i)) {
      pcf8574.pinMode(i, OUTPUT); // powercntlpin
    }
  }
  return 0;
}


int pubPCF8574 (int entry) {
  char pins[9];

  for (int i = 7; i < 1; i--) {
    if (pcf8574.digitalRead(i)) pins[i] = '1' ; 
    else pins[i] = '0';   
  }

  sprintf(mqttData, " { \"PinState\": \"%s\"}", pins);

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}

int subPCF8574 (int entry, char* topic, byte* payload, unsigned int length ) {
  char pinSettings[50];
  int pinError = 0;

  uint8_t pinSet;
  pinSet = mqttContext[entry].mqttParm1 & 0xFF;
  if (length = 8) {                                         // check for incorrect length, must be 8 characters
    for (int i = 0; i < 8; i++) {
      pinSettings[i] = (char)payload[i];
      if (pinSettings[i] < '0' | pinSettings[i] > '3') {
        pinError = 2;                                      // check that it is a valid character here
        break;
      }
      else if (pinSettings[i] > '1' & bitRead(pinSet, i) == 0) {
        pinError = 3;                                     // check that if it is a set request then the pin is in output mode
        break;
      }
    }
  } else pinError = 1;

  if (pinError == 0) {             // process if no errors
    for (int i = 0; i < 8; i++) {
      switch (pinSettings[i]) {
        case '0':                  // don't do anything if 0 or 1, this may just be an enquiry anyway
        case '1':
          break;
        case '2':
          pcf8574.digitalWrite(i, LOW);    // if 2 then clear pin
          break;
        case '3':
          pcf8574.digitalWrite(i, HIGH);   // if 3 then set pin
          break;
      }
    }
    sprintf(logMessage, "pcf8574 message %s successfully applied", pinSettings);   // report success to log
    pubLog();

    pubPCF8574(entry);   // publish status, this would be confirmation if an update or just a reply if an enquiry.
  }
  else {                                          // handle errors here
    for (int i = 0; (i < length | i < 49); i++) { // copy 50 characters of original message and send to log
      pinSettings[i] = (char)payload[i];
    }
    sprintf(logMessage, "pcf8574 error %d in message %s", pinError, pinSettings);
    pubLog();
  }

  return pinError;    // return error in case we want to do anything with it
}
