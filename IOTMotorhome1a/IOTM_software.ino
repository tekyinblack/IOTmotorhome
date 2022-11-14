/*
   Software device routines
*/

int pubSoftware (int entry) {
  displayAddress =  WiFi.localIP();
  sprintf(mqttData, " { \"Softparm1\": \"%d.%d.%d.%d\", \"Softparm2\" : \"%d\"}", 
           displayAddress[0],displayAddress[1],displayAddress[2],displayAddress[3],millis());

  

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}

int setupSoftware (int entry) {
}

int subSoftware (int entry, char* topic, byte* payload, unsigned int length ) {
  /*
   * This routine was created to test the software subscription and turn messages around
   */
  String softdata;
  for (int i = 0; i < length; i++) {
    softdata += (char)payload[i];
  }
  softdata.toCharArray(mqttData, (softdata.length() + 1));

  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
