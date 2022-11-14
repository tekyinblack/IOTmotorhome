  /*
   * epsolar routine
   * integrates solar controller over Modbus RS485 serial adapter
   * send two different reports at different intervals
   * live report is for graphical realtime updates
   * stats is for long term plotting using data accumulated on the epsolar controller
   */
   #include <ModbusMaster.h>
  ModbusMaster node;
/*
 * The following are complex data structures to map the information received from the controller as an 
 * serial stream to useable values. In each case, the 'data' structures hold the actual data and the 
 * 'maps'structure hold arrays into which the data streams are copied. The union statement maps these two 
 * data structures on top of each other allowing a simpler data conversion. In many case the actual values are 
 * stored as either 16 or 32 bit values with the actual value multiplied by 100, diving the values by 100 gives 
 * the correct value. 
 */
  // bool loadState = true;
  
  // epsolar register addresses in hex
  #define EPSOLAR_RTC 0x9013
  #define EPSOLAR_LIVE_LOWER 0x3100
  #define EPSOLAR_LIVE_MIDDLE 0x3110
  #define EPSOLAR_LIVE_UPPER 0x311A
  #define EPSOLAR_STATUS 0x3200
  // statistics registers, the clock registers are the same
   #define EPSOLAR_STATS_LOWER 0x3300
   #define EPSOLAR_STATS_UPPER 0x331B
  #define EPSOLAR_LOAD_STATE 0x02
  union {
    struct {
    uint8_t  second;   // 0x9013 R
    uint8_t  minute;
    uint8_t  hour;      // 0x9014 R
    uint8_t  Day;
    uint8_t  Month;  // 0x9015 R
    uint8_t  Year;
    
    int16_t  alignment;   // required to line up the word boundaries in memory to 32 bit boundary
    
    int16_t  panelVolts;   // 0x3100 R
    int16_t  panelCurrent; // 0x3101 R
    int32_t  panelPower; // 0x3102 R
    int16_t  batteryVoltsLive; // 0x3104
    int16_t  batteryCurrentLive; // 0x3105
    int32_t  batteryPower; // 0x3106
    uint16_t  dummy[4];   // 4 register addresses without data (future use or legacy?)
    int16_t  loadVolts;    // 0x310C R
    int16_t  loadCurrent; // 0x310D R
    int32_t  loadPower; // 0x310E R
    
    int16_t  batteryTemp;   // 0x3110 R
    int16_t  internalTemp;  // 0x3111
    int16_t  heatsinkTemp;  // 0x3112
    
    int16_t  batterySoc;  // 0x311A R
    int16_t  remoteBatteryTemp; // 0x311B
    
    int16_t  batteryStatus;  // 0x3200 R
    int16_t  chargingStatus;  // 0x3201 R
    
    int16_t  loadStatus;     // 0x2  R
 
    uint16_t  panelVoltsMax;        // 0x3300
    uint16_t  panelVoltsMin;        // 0x3301
    uint16_t  batteryVoltsMax;      // 0x3302
    uint16_t  batteryVoltsMin;      // 0x3303
    uint32_t  consumedEnergyToday;  // 0x3304
    uint32_t  consumedEnergyMonth;  // 0x3306
    uint32_t  consumedEnergyYear;   // 0x3308
    uint32_t  consumedEnergyTotal;  // 0x330A
    uint32_t  generatedEnergyToday; // 0x330C
    uint32_t  generatedEnergyMonth; // 0x330E
    uint32_t  generatedEnergyYear;  // 0x3310
    uint32_t  generatedEnergyTotal; // 0x3312
    uint32_t  c02Reduction;         // 0x3314
//    uint16_t alignment2;   // required to line up the word boundaries in memory
//    int16_t  batteryVoltage;      // 0x331A
    int32_t  batteryCurrent;        // 0x331B
    int16_t  batteryStatTemp;       // 0x331D
    int16_t  ambientTemp;           // 0x331E
   } data;
    struct {
    uint16_t transferRTC[3];   // 0x9013 3
    uint16_t alignment;         // required to line up the word boundaries in memory
    uint16_t transferLiveLower[16];  // 0x3100
    uint16_t transferLiveMiddle[3];  // 0x3110
    uint16_t transferLiveUpper[2];   // 0x311A
    uint16_t transferStatus[2];      // 0x3200
    uint16_t transferLoadStatus[1];  // 0x2
    uint16_t transferStatsLower[22];  // 0x3300
    uint16_t transferStatsUpper[4];   // 0x331A
//    uint16_t alignment2;   // required to line up the word boundaries in memory
//    uint16_t transferStatsUpper[5];   // 0x331A
   } maps;
    uint8_t buf[108];  
  } epsolarLive ;


/*
 *    Setup routine called from main setup  
 */
int setupEpsolar(int nodeNum) {
    Serial.begin(115200);
//    node.begin(nodeNum, Serial1);
    node.begin(1, Serial);
    return 0;
}

/*
 *  acquire and publish live data, split into two subroutines to enable easy changes of functionality
 */
int pubEpsolarLive(int entry) {
  readEpsolarLive();
  
  mqttEpsolarLive();
  client.publish(mqttContext[entry].mqttPubTopic, mqttData);
  return 0;
}
/*
 *  acquire and publish stats data, split into two subroutines to enable easy changes of functionality
 */
int pubEpsolarStats(int entry) {

  return 0;
}
/*
 * publish live solar data to mqtt, different formats available
 * 
 */
 void mqttEpsolarLive(void) {
      char dateBuf[18];
      int dataFormat=0;
      String epsolarLiveData;
     sprintf(dateBuf, "%02d-%02d-%02d %02d:%02d:%02d", epsolarLive.data.Year,epsolarLive.data.Month,epsolarLive.data.Day, 
                               epsolarLive.data.hour, epsolarLive.data.minute, epsolarLive.data.second);
      switch (dataFormat) {
        case 1:   // CSV format
             sprintf(mqttData, "%02d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%0X,%0X,%0X,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", 
                              epsolarLive.data.Year,
                              epsolarLive.data.Month,
                              epsolarLive.data.Day, 
                              epsolarLive.data.hour, 
                              epsolarLive.data.minute, 
                              epsolarLive.data.second,
                              epsolarLive.data.panelVolts/100.f,   // 3100
                              epsolarLive.data.panelCurrent/100.f, // 3101
                              epsolarLive.data.panelPower /100.f,  // 3102
                              epsolarLive.data.batteryVoltsLive/100.f,     // 331A
                              epsolarLive.data.batteryCurrent/100.f,     // 331B
                              epsolarLive.data.batteryVoltsMax/100.f,     // 3302
                              epsolarLive.data.batteryVoltsMin/100.f,    // 3303
                              epsolarLive.data.batteryTemp/100.f,    // 3110  
                              epsolarLive.data.batterySoc/1.f,       // 311A                                                  
                              epsolarLive.data.loadVolts/100.f,      // 310C
                              epsolarLive.data.loadCurrent/100.f,    // 310D
                              epsolarLive.data.loadPower/100.f,      // 310E
                              epsolarLive.data.internalTemp/100.f,   // 3111
                              epsolarLive.data.batteryStatus,        // 3200
                              epsolarLive.data.chargingStatus,       // 3201
                              epsolarLive.data.loadStatus,           // 02
                              epsolarLive.data.consumedEnergyToday/100.f,    // 3304
                              epsolarLive.data.consumedEnergyMonth/100.f,    // 3306
                              epsolarLive.data.consumedEnergyYear/100.f,     // 3308
                              epsolarLive.data.consumedEnergyTotal/100.f,    // 330A
                              epsolarLive.data.generatedEnergyToday/100.f,   // 330C
                              epsolarLive.data.generatedEnergyMonth/100.f,   // 330E
                              epsolarLive.data.generatedEnergyYear/100.f,    // 3310
                              epsolarLive.data.generatedEnergyTotal/100.f) ; // 3312                        
                             break;
          default:               // JSON format    
sprintf(mqttData, "{\"datetime\": \"%02d-%02d-%02d %02d:%02d:%02d\
\", \"panelVolts\" : \"%.2f\", \"panelCurrent\" : \"%.2f\", \"panelPower\" : \"%.2f\
\", \"batteryVolts\" : \"%.2f\", \"batteryCurrent\" : \"%.2f\
\", \"batteryVoltsMax\" : \"%.2f\", \"batteryVoltsMin\" : \"%.2f\
\", \"batteryTemp\" : \"%.2f\", \"batterySoc\" : \"%.2f\
\", \"loadVolts\" : \"%.2f\", \"loadCurrent\" : \"%.2f\", \"loadPower\" : \"%.2f\
\", \"internalTemp\" : \"%.2f\
\", \"batteryStatus\" : \"%0X\", \"chargingStatus\" : \"%0X\", \"loadStatus\" : \"%0X\
\", \"consumedEnergyToday\" : \"%.2f\", \"consumedEnergyMonth\" : \"%.2f\
\", \"consumedEnergyYear\" : \"%.2f\", \"consumedEnergyTotal\" : \"%.2f\
\", \"generatedEnergyToday\" : \"%.2f\", \"generatedEnergyMonth\" : \"%.2f\
\", \"generatedEnergyYear\" : \"%.2f\", \"generatedEnergyTotal\" : \"%.2f\"}", 
                              epsolarLive.data.Year,
                              epsolarLive.data.Month,
                              epsolarLive.data.Day, 
                              epsolarLive.data.hour, 
                              epsolarLive.data.minute, 
                              epsolarLive.data.second,
                              epsolarLive.data.panelVolts/100.f,   // 3100
                              epsolarLive.data.panelCurrent/100.f, // 3101
                              epsolarLive.data.panelPower /100.f,  // 3102
                              epsolarLive.data.batteryVoltsLive/100.f,     // 331A
                              epsolarLive.data.batteryCurrent/100.f,     // 331B
                              epsolarLive.data.batteryVoltsMax/100.f,     // 3302
                              epsolarLive.data.batteryVoltsMin/100.f,    // 3303
                              epsolarLive.data.batteryTemp/100.f,    // 3110  
                              epsolarLive.data.batterySoc/1.f,       // 311A                                                  
                              epsolarLive.data.loadVolts/100.f,      // 310C
                              epsolarLive.data.loadCurrent/100.f,    // 310D
                              epsolarLive.data.loadPower/100.f,      // 310E
                              epsolarLive.data.internalTemp/100.f,   // 3111
                              epsolarLive.data.batteryStatus,        // 3200
                              epsolarLive.data.chargingStatus,       // 3201
                              epsolarLive.data.loadStatus,           // 02
                              epsolarLive.data.consumedEnergyToday/100.f,    // 3304
                              epsolarLive.data.consumedEnergyMonth/100.f,    // 3306
                              epsolarLive.data.consumedEnergyYear/100.f,     // 3308
                              epsolarLive.data.consumedEnergyTotal/100.f,    // 330A
                              epsolarLive.data.generatedEnergyToday/100.f,   // 330C
                              epsolarLive.data.generatedEnergyMonth/100.f,   // 330E
                              epsolarLive.data.generatedEnergyYear/100.f,    // 3310
                              epsolarLive.data.generatedEnergyTotal/100.f) ; // 3312        
                                                      
                               break;
      }                     
  //    epsolarLiveData.toCharArray(mqttData, (epsolarLiveData.length() + 1));                            
 }
/*
 * Use modbus protocol to access epsolar controller over serial connection 
 * to acquire live data
 */

void readEpsolarLive(void) {
  
  memset(epsolarLive.buf,0,sizeof(epsolarLive.buf));
  
  node.clearResponseBuffer();
  int dataSize = sizeof(epsolarLive.maps.transferRTC)/2;
  int result = node.readHoldingRegisters(EPSOLAR_RTC, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferRTC[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar Live rtc-data read  %0X",result);
      pubLog();
  } 
  
  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferLiveLower)/2;
  result = node.readInputRegisters(EPSOLAR_LIVE_LOWER, 16);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< 16 ;i++) epsolarLive.maps.transferLiveLower[i] = node.getResponseBuffer(i); 
     } else {
      sprintf(logMessage, "epsolar Live Lower read  %0X",result);
      pubLog();
  }   

  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferLiveMiddle)/2;
  result = node.readInputRegisters(EPSOLAR_LIVE_MIDDLE, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferLiveMiddle[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar Live Middle read  %0X",result);
      pubLog();
  }  
  
  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferLiveUpper)/2;
  result = node.readInputRegisters(EPSOLAR_LIVE_UPPER, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferLiveUpper[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar Live Upper read  %0X",result);
      pubLog();      
  }   
  
  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferStatus)/2;
  result = node.readInputRegisters(EPSOLAR_STATUS, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferStatus[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar Live Status read  %0X",result);
      pubLog();       
  }  

   node.clearResponseBuffer();
   result = node.readCoils(EPSOLAR_LOAD_STATE, 1 );
   if (result == node.ku8MBSuccess)  { 
         epsolarLive.maps.transferLoadStatus[0] = node.getResponseBuffer(0);
        
  } else  {
      sprintf(logMessage, "epsolar Live Load State read  %0X",result);
      pubLog();
  }
  
  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferStatsLower)/2;
  result = node.readInputRegisters(EPSOLAR_STATS_LOWER, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferStatsLower[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar StatsLower read  %0X",result);
      pubLog();       
  }   
    
  node.clearResponseBuffer();
  dataSize = sizeof(epsolarLive.maps.transferStatsUpper)/2;
  result = node.readInputRegisters(EPSOLAR_STATS_UPPER, dataSize);
  
  if (result == node.ku8MBSuccess)  {
        for(int i=0; i< dataSize ;i++) epsolarLive.maps.transferStatsUpper[i] = node.getResponseBuffer(i);
     } else {
      sprintf(logMessage, "epsolar StatsUpper read  %0X",result);
      pubLog();
  }   
}
  

 
