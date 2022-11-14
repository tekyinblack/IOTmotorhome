/*
 * log message routine
 */

void pubLog() {
      sprintf(mqttData, " { \"MsgTime\": \"%d\", \"MsgText\" : \"%s\"}", 
           millis(),logMessage);
  
      client.publish("espLog", mqttData);

  }
