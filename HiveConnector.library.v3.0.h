#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <SPI.h>
#include <Wire.h>
#include <time.h>



/*
 * MQTT Library Required. PubSubClient MQTT
 * https://pubsubclient.knolleary.net/
 * Important , MaxLimit on the 
 * https://github.com/knolleary/pubsubclient#limitations
 * https://github.com/knolleary/pubsubclient/issues/62 
 *  The maximum message size, including header, is 128 bytes by default. 
 *  This is configurable via MQTT_MAX_PACKET_SIZE in PubSubClient.h.
 *  
 *  below did not work even after changing PubSub...h
 *  #define MQTT_MAX_PACKET_SIZE 450 
 */
 
#include <PubSubClient.h>

#include <DoubleResetDetector.h>
// Number of seconds after reset during which a subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10
// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);



bool _shouldSaveConfigToFile = false;
void _saveConfigCallback () {
  Serial.println("INFO : [HIVEBOT] Recieved Config from AP (Soft Access Point Mode)");
  _shouldSaveConfigToFile = true;
}

void _configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("INFO : [HIVEBOT] Entering ConfigMode: AP (Soft Access Point Mode)");
  Serial.print  ("INFO : [HIVEBOT]    > Connect to Soft Wifi AccessPoint: " );
  Serial.println(bot_accessPointName);
  Serial.print  ("INFO : [HIVEBOT]    > Configure at http://");
  Serial.println(WiFi.softAPIP() );
  
  drd.stop(); //Reset , so next boot does not go into SoftAP Config Mode.
}


void setupHiveConnector() {
  if (!SPIFFS.begin()) {
    Serial.println("ERRO : [HIVEBOT] Unable to mount FS. ");
    return;
  }


  loadConfigFromFile(); //Load any Previous Config.
  
  WiFiManager wifiManager;
  wifiManager.setAPCallback(_configModeCallback);
  wifiManager.setSaveConfigCallback(_saveConfigCallback); 
  wifiManager.setDebugOutput(WIFI_MANAGER_DEBUG);

  WiFiManagerParameter mqtt_serverWiFiConfig("mqtt_server", "192.168.1.200", config_mqtt_server, 50);
  WiFiManagerParameter mqtt_server_portWiFiConfig("mqtt_server_port", "1883", config_mqtt_server_port, 5);
  WiFiManagerParameter mqtt_userWiFiConfig("mqtt_user", "<username>", config_mqtt_user, 50);
  WiFiManagerParameter mqtt_pswdWiFiConfig("mqtt_pswd", "<password>", config_mqtt_pswd, 50);
  wifiManager.addParameter(&mqtt_serverWiFiConfig);
  wifiManager.addParameter(&mqtt_server_portWiFiConfig);
  wifiManager.addParameter(&mqtt_userWiFiConfig);
  wifiManager.addParameter(&mqtt_pswdWiFiConfig);
  
   if (drd.detectDoubleReset()) {
    Serial.println("INFO : [HIVEBOT] Hard Reset Detected. Forcing to ConfigMode: AP (Soft Access Point Mode)");
    wifiManager.startConfigPortal(bot_accessPointName, "");
   }else{
    Serial.println("INFO : [HIVEBOT] AutoConnect [ AP (Soft Access Point Mode) _or_ ST (Station Mode as WifiClient) ]" );
    wifiManager.autoConnect(bot_accessPointName, "");
   }
    //if(true) return;
   strcpy(config_mqtt_server, mqtt_serverWiFiConfig.getValue());
   strcpy(config_mqtt_server_port, mqtt_server_portWiFiConfig.getValue());
   strcpy(config_mqtt_user, mqtt_userWiFiConfig.getValue());
   strcpy(config_mqtt_pswd, mqtt_pswdWiFiConfig.getValue());

   //if (_shouldSaveConfigToFile) {  , Value not being set, callback not working. Skipping.
    saveConfigToFile();
  //}
  drd.stop();

  Serial.println("INFO : [HIVEBOT] Ready running ST (Station Mode as WifiClient) ]");
  Serial.print  ("INFO : [HIVEBOT]    > Connected to Wifi: ");
  Serial.println(WiFi.SSID() );
  Serial.print  ("INFO : [HIVEBOT]    > My IP: ");
  Serial.println(WiFi.localIP() );
}





void forceHardResetToConfigMode() {
  Serial.println("INFO : [HIVEBOT] HardReset Initiated, will start ConfigMode: AP (Soft Access Point Mode)");
  WiFi.disconnect();
  delay(500);
  ESP.restart();
  delay(5000);
}

/* MQTT Connectivity Setup */


/* Callback Methods , you can overide in client Implementation */
void callbackMqttConnected();
void callbackMqttNotConnected();
void callbackUpdateFunctions(String enabledFunctions);
void callbackInstructionRecieved(long instrId,String command, String params);


void callbackMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("DEBUG: [MQTT] Message Recieved[  < < < ]:");
  String strPayload = "";
  for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    strPayload +=receivedChar;
  }
  Serial.println(strPayload);
  StaticJsonBuffer<1000> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(payload); //Parse message
  if (!parsed.success()) {   //Check for errors in parsing
    Serial.println("ERROR: [RECV] JSON Parsing failed");
  }else{
    const char* hiveBotId    = parsed["hiveBotId"];
    if(String(hiveBotId) != bot_id){
      Serial.println("INFO : [RECV] Ignoring, not a Message for me.");
      return ;
    }
    boolean dataTypeNotUnderstood = true;
    const char* dataType    = parsed["dataType"];
    if (String(dataType) == "UpdateFunctions" || String(dataType) == "CatchupPostBootup") {
      const char* enabledFunctions    = parsed["enabledFunctions"];
      callbackUpdateFunctions(String(enabledFunctions));
      dataTypeNotUnderstood=false;
    }
    
    if(String(dataType) == "ExecuteInstruction" || String(dataType) == "CatchupPostBootup"){
      JsonArray& instructions = parsed["instructions"];
      for(unsigned  int iCount=0;iCount<instructions.size();iCount++){
          JsonVariant instJsonVariant = instructions.get<JsonVariant>(iCount);
          JsonObject &instJsonO = instJsonVariant.as<JsonObject>();
          long instrId = instJsonO["instrId"];
          const char* command = instJsonO["command"];
          const char* schedule = instJsonO["schedule"];
          const char* params = instJsonO["params"];
          const char* executenow = instJsonO["execute"];
          if (strcasecmp(executenow,"true")==0) {
            callbackInstructionRecieved(instrId,String(command),String(params));
          }
      } 
      dataTypeNotUnderstood=false;
    }
      /*else if(String(dataType) == "UpdateFunctions"){
      const char* enabledFunctions    = parsed["enabledFunctions"];
      callbackUpdateFunctions(String(enabledFunctions));*/
    if(dataTypeNotUnderstood) {
      Serial.print("ERROR: [RECV] Unknown DataType Ignoring.");
      Serial.println(dataType);
    }
  }
  
  /*
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  //client.publish("outTopic", p, length);
  String recvMsg = String((char *)p);
  Serial.println(recvMsg);
  //callbackMqttMessageRecieved(recvMsg);
  // Free the memory
  free(p);
  */
}
WiFiClient wifiClient;
PubSubClient client(config_mqtt_server, mqtt_server_port, callbackMqttMessage, wifiClient);
boolean mqttConnected =false;
EventTimer mqttReconnectOnFailTimer("MQTTReconnect#",1000 * 2, true, false); //every x seconds
boolean _isMQTTConnected(){
  mqttConnected = client.connected();
  if(mqttConnected){
    return true;
  }
  //Give a bit of breather when reconnecting.
  if(!mqttReconnectOnFailTimer.isDueForRun()) 
    return false;

  Serial.print("DEBUG: [MQTT] settings:[");
  Serial.print(config_mqtt_server);Serial.print(", ");
  Serial.print(config_mqtt_user);Serial.print(", ");
  Serial.print(config_mqtt_pswd);Serial.print(", ");
  Serial.println("] ");
  
  if (client.connect(mqtt_microclima_id,config_mqtt_user,config_mqtt_pswd)) {
    Serial.println("DEBUG: [MQTT] Connected to Broker"); 
    Serial.print("DEBUG: [MQTT] Subscribing to : ");
    Serial.println(mqtt_botcli_recieve_topic);
    Serial.print("DEBUG: [MQTT] Subscribing to : ");
    Serial.println(mqtt_botcli_recieve_retained_will_topic);
    client.subscribe(mqtt_botcli_recieve_topic,mqtt_subscribe_qos);
    client.subscribe(mqtt_botcli_recieve_retained_will_topic);
    mqttConnected = client.connected();
    if(mqttConnected){
      Serial.println("DEBUG: [MQTT] Connected.");
      callbackMqttConnected(); 
    }else{
      Serial.println("DEBUG: [MQTT] Connection Failure.");
    }
  }
  else {
    Serial.println("DEBUG: [MQTT] Broker Connection Failed. (username, password)");
    callbackMqttNotConnected();
    mqttConnected=false;
    //abort();
  }

}

boolean isHiveConnected(){
  if(!_isMQTTConnected()) return false;
  return true;
}

void loopHiveConnector(){
  if(!isHiveConnected()) return ;
  client.loop();
}

void disconnectFromHive(){
  if(isHiveConnected()){
    client.disconnect();
    mqttReconnectOnFailTimer.enabled(false);
    Serial.println("DEBUG: [MQTT] Disconnected from Broker and turning off AutoReconnect");
  }
}

const static int DATATYPE_NOTHING_SPECIAL_BUT_LET_THEM_KNOW_I_AM_ALIVE=1;
const static int DATATYPE_BOOTUP_NOTIFY=2;
const static int DATATYPE_SENSOR_DATA=200;
const static int DATATYPE_INSTRUCTION_COMPLETED=201;
const static int DATATYPE_INSTRUCTION_EXEFAILED=501;
//SensorData
boolean publishToHive(int dataTypeFor, String messageSetJson){
  
  if(!isHiveConnected()) return false;
  else {
    
    String requestPayload = "{";
    if(dataTypeFor == DATATYPE_SENSOR_DATA){
      requestPayload += "\"dataType\": \"SensorData\"";
      requestPayload += ",\"dataMap\": {";
      requestPayload += messageSetJson;
      requestPayload += "}";
    }else if(dataTypeFor == DATATYPE_INSTRUCTION_COMPLETED){
      requestPayload += "\"dataType\": \"InstructionCompleted\"";
      requestPayload += ",\"instructions\": [";
      requestPayload += messageSetJson;
      requestPayload += "]";
    }else if(dataTypeFor == DATATYPE_INSTRUCTION_EXEFAILED){
      requestPayload += "\"dataType\": \"InstructionFailed\"";
      requestPayload += ",\"instructions\": [";
      requestPayload += messageSetJson;
      requestPayload += "]";
    }else if(dataTypeFor == DATATYPE_BOOTUP_NOTIFY){
      requestPayload += "\"dataType\": \"BootupHivebot\"";
    }else {
      requestPayload += "\"dataType\": \"HeartBeat\"";
    }
    requestPayload += ",\"hiveBotId\": \"";
    requestPayload += bot_id;
    requestPayload += "\",\"accessKey\": \"";
    requestPayload += hive_accesskey;
    requestPayload += "\"";
    requestPayload += "}";

    
    if(client.publish(mqtt_controller_notify_topic,(char*) requestPayload.c_str())){
    //if(client.publish("AAAA",)){
      Serial.print("DEBUG: [MQTT] Message Published[ > > > ]:");
      Serial.println(requestPayload);
    }else{
      Serial.println("DEBUG: [MQTT] Error Publishing Message");
    }
  }
}





