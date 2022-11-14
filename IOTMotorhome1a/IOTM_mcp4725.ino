/**************************************************************************/
/*!
   This is an example sketch for the Adafruit MCP4725 breakout board
    ----> http://www.adafruit.com/products/935

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

// Set this value to 9, 8, 7, 6 or 5 to adjust the resolution
#define DAC_RESOLUTION    (8)

/* Note: If flash space is tight a quarter sine wave is enough
   to generate full sine and cos waves, but some additional
   calculation will be required at each step after the first
   quarter wave.                                              */

int setupMCP4725(int entry) {


  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x60);


}

int subMCP4725(int entry, char* topic, byte* payload, unsigned int length ) {
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
    dac.setVoltage(outVoltage, false);
  
    sprintf(logMessage, "mcp4725 message %s successfully applied", setVoltage);   // report success to log
    pubLog();

  }
  else {                                          // handle errors here
    for (int i = 0; (i < length | i < 49); i++) { // copy 50 characters of original message and send to log
      setVoltage[i] = (char)payload[i];
    }
    sprintf(logMessage, "mcp4725 error %d in message %s", pinError, setVoltage);
    pubLog();
  }

  return pinError;    // return error in case we want to do anything with it
}
