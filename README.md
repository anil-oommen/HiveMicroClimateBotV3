
Important , current PubSub does not allow the messags to be stored on Broker. 
As this requires some tracking in the IOT which should be persisted. 
As a resuult when the IOt is in DeepSleep any notification from Server is lost.
https://github.com/knolleary/pubsubclient/issues/200


https://myesp8266.blogspot.sg/2016/12/bmp280-and-esp8266.html
http://arduinoprojects.uk/iot-esp8266-barometer-using-bmp280/
https://learn.adafruit.com/assets/26858

http://192.168.1.103:8080/swagger-ui.html#!/bot-controller/registerUsingPOST

iot.microcli.bot
save
{  
   "hiveBotId":"OOMM.HIVE MICLIM.01",
"enabledFunctions": "IR_LISTEN,MQTT,DHT22",
"hiveInstructions": "LEDDANCE",
"accessKey":"3cfe3256ba8b7a54b464370c68f59b6352d9907979bb8ab037e5da9f0ff7a23d"
}


{"hiveBotId":"OOMM.HIVE MICLIM.01","accessKey":"3cfe325"}

Instructions 
REBOOT
IRAC_OFF

{"hiveBotId":"OOMM.HIVE MICLIM.01","accessKey":"3cfe325","infoType":"SensorData","dataMap": { } }


  String requestPayload = "{\"hiveBotId\": \"";
    requestPayload += bot_id;
    requestPayload += "\",\"accessKey\": \"";
    requestPayload += hive_accesskey;
    requestPayload += "\",\"dataMap\": {";
    if(includeDataMap){
      requestPayload += hiveDataMapJSONContent;
    }
    requestPayload += "}}";
	
{"hiveBotId": "Controller","accessKey": "0","dataType": "DataRetrieve"}