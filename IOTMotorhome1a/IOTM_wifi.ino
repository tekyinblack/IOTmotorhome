/*
 * 
 */

void setup_wifi() {
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.print(".");
  }

}




void callback(char* topic, byte* payload, unsigned int length) {

  for (int i=0; i<itemCount; i++) {   
     if (mqttContext[i].mqttElig == 1 && mqttContext[i].mqttSub == 1) {
        if (strcmp(mqttContext[i].mqttSubTopic, topic) == 0) {
            mqttContext[i].mqttSubFunction(i, topic, payload, length);
            }
        }
     } 
  for (int i = 0; i < length; i++) {
  //  Serial.print((char)payload[i]);
  }


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
 //   Serial.print("Attempting MQTT connection...");
    
//    // Create a random client ID
//    String clientId = "ESP8266Client-";
//    clientId += String(random(0xffff), HEX);
//    // Attempt to connect
//    if (client.connect(clientId.c_str())) {

    if (client.connect(mqtt_client)) {    
  //    Serial.println("connected");
      
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      // client.subscribe("inTopic");
      
        for (int i=0; i<itemCount; i++) {   
          
           if (mqttContext[i].mqttElig == 1 && mqttContext[i].mqttSub == 1) {
            
             client.subscribe(mqttContext[i].mqttSubTopic);
        
             }
           } 
    } else {
  //    Serial.print("failed, rc=");
 //    Serial.print(client.state());
 //     Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
