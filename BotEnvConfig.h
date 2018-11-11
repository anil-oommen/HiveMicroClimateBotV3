#include "FS.h"
#include <ArduinoJson.h>

#define WIFI_MANAGER_DEBUG false
#define SPIFFS_CONFIG_JSONFILE "/config.json"
#define HIVE_BOT_ID "HIVEBOT_MICLIM.03"
#define HIVE_BOT_VERSION "v3.0"
#define HIVE_BOT_ACCESSKEY "1b4b882772c"
#define HIVE_BOT_MQTTCLIENT_ID "miclim_esp8266_v3"

/* Config Settings from the WifiManager @ Wifi Setup.*/
char config_mqtt_server[50] = "";
char config_mqtt_server_port[5] = "1883";
char config_mqtt_user[50] = "";
char config_mqtt_pswd[50] = "";


/* MQTT Connection Settings ------------- */
int         mqtt_server_port = 1883;  //TODO hardcoded for now, not copied from config_mqtt_server_port
const char* mqtt_microclima_id = HIVE_BOT_MQTTCLIENT_ID;
int         mqtt_subscribe_qos = 1;

const char* mqtt_controller_notify_topic =                "hivecentral/controller/microclimate";
const char* mqtt_botcli_recieve_topic =                   "hivecentral/botclients/microclimate";
const char* mqtt_botcli_recieve_retained_will_topic =     "hivecentral/botclients/retainedwill/microclimate";

/* BOT Details --------------- */
const char* bot_accessPointName  = HIVE_BOT_ID;
String bot_id = HIVE_BOT_ID;
String bot_desc = "MicroClimate BOT@IOT HiveCentral";
const char bot_compile_date[] = __DATE__ " " __TIME__;
String bot_version = HIVE_BOT_VERSION;
String hive_accesskey = HIVE_BOT_ACCESSKEY;



bool loadConfigFromFile() {
  File configFile = SPIFFS.open(SPIFFS_CONFIG_JSONFILE, "r");
  if (!configFile) {
    Serial.println("ERRO : [HIVEBOT] Unable to load config File. ");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("ERRO : [HIVEBOT] Config file size overflow.");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("ERRO : [HIVEBOT] Unable to parse config json.");
    return false;
  }

  strcpy(config_mqtt_server, json["config_mqtt_server"]);
  strcpy(config_mqtt_server_port, json["config_mqtt_server_port"]);
  strcpy(config_mqtt_user, json["config_mqtt_user"]);
  strcpy(config_mqtt_pswd, json["config_mqtt_pswd"]);

  Serial.print("DEBUG: [CONFIG] Loaded from Json[");
  Serial.print(config_mqtt_server);Serial.print(", ");
  Serial.print(config_mqtt_user);Serial.print(", ");
  Serial.print(config_mqtt_pswd);Serial.print(" ");
  Serial.println("] ");
  
  return true;
}

bool saveConfigToFile() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["config_mqtt_server"] = config_mqtt_server;
  json["config_mqtt_server_port"] = config_mqtt_server_port;
  json["config_mqtt_user"] = config_mqtt_user;
  json["config_mqtt_pswd"] = config_mqtt_pswd;

  Serial.print("DEBUG: [CONFIG] Saving to Json[");
  Serial.print(config_mqtt_server);Serial.print(", ");
  Serial.print(config_mqtt_user);Serial.print(", ");
  Serial.print(config_mqtt_pswd);Serial.print(" ");
  Serial.println("] ");
  

  File configFile = SPIFFS.open(SPIFFS_CONFIG_JSONFILE, "w");
  if (!configFile) {
    Serial.println("ERRO : [HIVEBOT] Unable to save config file");
    return false;
  }
  json.printTo(configFile);
  return true;
}
