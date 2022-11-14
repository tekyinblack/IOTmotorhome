#include <Adafruit_PCF8591.h>
// Make sure that this is set to the value in volts of VCC
#define ADC_REFERENCE_VOLTAGE 5.0
Adafruit_PCF8591 pcf = Adafruit_PCF8591();

int setupPCF8591(int entry) {
setupWire();     // setup wire if not already done

  if (!pcf.begin()) {
    return 1;
  }
  pcf.enableDAC(true);
   return 0;
}

int pubPCF8591(int entry) {

   sprintf(mqttData, " { \"AIN0\": \"%d\",\"AIN1\" : \"%d\",\"AIN2\":\"%d\",\"AIN3\" : \"%d\"}", 
           pcf.analogRead(0),
           pcf.analogRead(1),
           pcf.analogRead(2),
           pcf.analogRead(3));

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;

}

int subPCF8591(int entry, char* topic, byte* payload, unsigned int length ) {
  int maxChars=5;
  char setVoltage[6];
  int pinError = 0;
  int outVoltage = 0;

  if (length <= 5) {                                         // check for incorrect length, must be 8 characters
    for (int i = 0; i < length; i++) {
      setVoltage[i] = (char)payload[i];
      if (setVoltage[i] < '0' | setVoltage[i] > '9') {
        pinError = 2;                                      // check that it is a valid character here
        break;
      }
    }
    setVoltage[length] = 0;
  } else pinError = 1;

  if (pinError == 0) {             // process if no errors
    outVoltage = atoi(setVoltage);
    pcf.analogWrite(outVoltage);
  
    sprintf(logMessage, "pcf8591 message %s successfully applied", setVoltage);   // report success to log
    pubLog();

  }
  else {                                          // handle errors here
    for (int i = 0; (i < length | i < 49); i++) { // copy 50 characters of original message and send to log
      setVoltage[i] = (char)payload[i];
    }
    sprintf(logMessage, "pcf8591 error %d in message %s", pinError, setVoltage);
    pubLog();
  }

  return pinError;    // return error in case we want to do anything with it 
  
}
