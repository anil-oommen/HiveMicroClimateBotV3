
/* 
 *  HiveCentral BOT for MicroClimate Sensors and AirCon Controller over IR.
 *  MicroController ESP8266 12e NodeMCU. 
 *  Connected Components :
 *  -- LED Connected RED ( +v=D1, -v=:Resistory:GND25 )
 *  -- LED Connected GREEN ( +v=D2, -v=:Resistory:GND25 )
 *  
 *  Features :
 *  -- 
 *  Change To Your Environment Settings, refer "HiveBotParams.default.library.dont.change.h" 
 */

#include "BotEnvConfig.private.h" 
#include "LEDNotify.library.v2.0.h"
#include "HiveUtility.library.v2.0.h"
#include "BotSensors.library.v2.0.h"
#include "HiveConnector.library.v3.0.h"
#include "IRAirconRemote.utility.h"

/*
 * Our EventTimes and Enabled Switches
 */

EventTimer heartbeatTimer("HeartBeat",      1000 * 60 * 5 , true,   false); //every x mins
EventTimer sensorTimer("Sensor",            1000 * 60  , true,   true);  //every x seconds, Run from bootuptime or when enabled; Use
EventTimer irRecieverFunction("IRReciever", 500       , false,  false); //How frequent we should give control
EventTimer deepsleepFunction("Deepsleep",   1000 * 10 , false,  false); //every x Seconds , No need to run immediate if enabled. Give time for others.
/* 
 * Overide HiveConnector Callback  
 * 
*/
void callbackMqttConnected(){
  publishToHive(DATATYPE_BOOTUP_NOTIFY,"");
}
void callbackMqttNotConnected(){}
void callbackUpdateFunctions(String enabledFunctions){
  Serial.print("DEBUG: [FUNCT] +DHT22    : " );
  String ifOnRunFreq = "ON (check_frequency_secs) " + String(sensorTimer.runFrequency()/1000);
  Serial.println((sensorTimer.enabled((enabledFunctions.indexOf("DHT22") > 0)))?ifOnRunFreq:"OFF");

  Serial.print("DEBUG: [FUNCT] +DEEPSLEEP: " );
  ifOnRunFreq = "ON (after_secs) " + String(deepsleepFunction.runFrequency()/1000);
  Serial.println((deepsleepFunction.enabled((enabledFunctions.indexOf("DEEPSLEEP") > 0)))?ifOnRunFreq:"OFF");

  Serial.print("DEBUG: [FUNCT] +IR_LISTEN: " );
  Serial.println((irRecieverFunction.enabled((enabledFunctions.indexOf("IR_LISTEN") > 0)))?"ON":"OFF");

  
}
void callbackInstructionRecieved(long instrId,String command, String params){
  boolean publishInstructionSucessfull = false;
  boolean publishInstructionFailed = false;
  boolean rebootDeviceAfterReportingToServer = false;
  if(command == "LEDDANCE"){
    Serial.print("DEBUG: [LEDDANCE] Executing. InstructionId:" );
    Serial.println(instrId);
    doLEDDance();
    publishInstructionSucessfull=true;
  }else if(command == "REBOOT"){
    Serial.print("DEBUG: [REBOOT] Executing. InstructionId:" );
    Serial.println(instrId);
    doLEDDance();
    publishInstructionSucessfull=true;
    rebootDeviceAfterReportingToServer=true;
    
  }else if(command == "IRAC_OFF"){
    Serial.print("DEBUG: [IRAC_OFF] Executing. InstructionId:" );
    Serial.println(instrId);
    setAirconProfile(0,false,26,KELVINATOR_COOL,0);
    sendAirconProfile();
    publishInstructionSucessfull=true;
    publishToHive(DATATYPE_SENSOR_DATA,getAirconfProfileDataMap());
  }else if(command == "IRAC_ONN_PROFILE_A"){
    Serial.print("DEBUG: [IRAC_ONN_PROFILE_A] Executing. InstructionId:" );
    Serial.println(instrId);
    setAirconProfile(1,true,24,KELVINATOR_COOL,2);
    sendAirconProfile();
    publishInstructionSucessfull=true;
    publishToHive(DATATYPE_SENSOR_DATA,getAirconfProfileDataMap());
  }else if(command == "IRAC_ONN_PROFILE_B"){
    Serial.print("DEBUG: [IRAC_ONN_PROFILE_B] Executing. InstructionId:" );
    Serial.println(instrId);
     setAirconProfile(2,true,24,KELVINATOR_DRY,2);
     sendAirconProfile();
    publishInstructionSucessfull=true;
    publishToHive(DATATYPE_SENSOR_DATA,getAirconfProfileDataMap());
  }else if(command == "IRAC_ONN_PROFILE_C"){
    Serial.print("DEBUG: [IRAC_ONN_PROFILE_C] Executing. InstructionId:" );
    Serial.println(instrId);
    setAirconProfile(3,true,24,KELVINATOR_FAN,4);
    sendAirconProfile();
    publishInstructionSucessfull=true;
    publishToHive(DATATYPE_SENSOR_DATA,getAirconfProfileDataMap());
  }else{
    Serial.print("DEBUG: [UNKINSR] Unknown Instruction no action taken:" );
    Serial.print(instrId);
    Serial.println(command);
  }

  if(publishInstructionSucessfull){
    String insrDet = "{\"instrId\":" ;
    insrDet += String(instrId);
    insrDet += ",\"command\":\"";
    insrDet += command;
    insrDet += "\"}";
    publishToHive(DATATYPE_INSTRUCTION_COMPLETED,insrDet);
  }else if(publishInstructionFailed){
    String insrDet = "{\"instrId\":" ;
    insrDet += String(instrId);
    insrDet += ",\"command\":\"";
    insrDet += command;
    insrDet += "\"}";
    publishToHive(DATATYPE_INSTRUCTION_EXEFAILED,insrDet);
  }


  if(rebootDeviceAfterReportingToServer){
    disconnectFromHive();
    Serial.println("DEBUG: [REBOOT] Rebooting Device in 5 seconds" );
    delay(1000 * 5);
    ESP.deepSleep(3e6); // 10e6 = 10 Seconds, 
  }
}

void loop() 
{
  
  loopHiveConnector();
  
  /*greenLEDBlinkLong(1);
  redLEDBlinkLong(1);
  Serial.println("Blink");
  readSensors();
  if(loopAndCheckHiveConnected()){
    //Send MQTT Connection.s
    String sampleMessage = "T00.";
    sampleMessage +=String(counter);
    publishToHive(sampleMessage);
    counter++;
  }
  delay(3000);
  */
  if(isHiveConnected()){

    //String sampleMessage = "T00.";
    //sampleMessage +=String(pubTimer.runCounts());

    //Check time to collect Sensor Data
    if(sensorTimer.isDueForRun()){
      if(readSensors()){
        String dataMap = "\"Temperature\": \""+ String(dht22_temp_f) +"\""  ;
        dataMap += ",\"HumidityPercent\": \""+ String(dht22_humidity) +"\""  ;
        dataMap += ",\"DHT22_SensorStatus\": \"OK\""  ;
        publishToHive(DATATYPE_SENSOR_DATA,dataMap);
      }else{
        String dataMap = "\"Temperature\": \"-1\" ,\"HumidityPercent\": \"-1\", \"DHT22_SensorStatus\": \"Error Reading DHT22.\""  ;
        publishToHive(DATATYPE_SENSOR_DATA,dataMap);
      }  
    }else if(deepsleepFunction.isDueForRun()){
      disconnectFromHive();
      Serial.println("DEBUG: [DEEPSLEEP] Going into PowerSaver Sleep." );
      delay(1000 *1);
      ESP.deepSleep(180e6); // 10e6 = 10 Seconds , freq in MicroSeconds, 
    }else if(heartbeatTimer.isDueForRun()){
      //if Nothing Else to Publish , just a heartBeat since its pubTime
      String dataMap = "";
      publishToHive(DATATYPE_NOTHING_SPECIAL_BUT_LET_THEM_KNOW_I_AM_ALIVE,dataMap);
    }else if(irRecieverFunction.isDueForRun()){
      //
      Serial.printf("DEBUG: [IR_RECIEVE] Handling Control to IR Reader for %d Seconds.\n", irContinusRunForSecs);
      while(!checkIRAndInteruptForOtherProcessing()){
        //Keep going Man.
      }
      if(!irDataPayload.equals("") ){
        if(irDataPayload.length()>50){
          String dataMap = "\"IRemoteData\": \"" + irDataPayload +"\""  ;
          publishToHive(DATATYPE_SENSOR_DATA,dataMap);
        }else{
          Serial.println("WARN : [IR_RECIEVE] IR Data Ignored < 50 Chars, Possibly Interference");
        }
      }
    }

  }
  delay(100);
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("INFO : [HIVEBOT] Booting up.");
  //Serial.println("Setup Done");
  //prntBanner_Bootup();
  delay(10);
  setupLEDNotify();
  setupHiveConnector();
  //Connected to Wifi Post AP Setup.
  //Setup 
  setupIRModule();
}

//void loop(){}

