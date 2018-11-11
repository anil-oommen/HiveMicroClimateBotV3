


/*  DHT22 , Sensor 1
 *  Library Required 'Adafruit_Unified_Sensor'
 *  
 */
#include <DHT.h>      
#define DHTTYPE DHT22       
#define DHTPIN  2

/* DHT 22 Sensor Information ------------------------- */
DHT dht(D5, DHTTYPE);


// Values read from sensor
boolean dht22_active =false;
float dht22_humidity, dht22_temp_f;  
boolean _readDht22Sensors(){
  dht22_humidity = dht.readHumidity();          // Read humidity (percent)
  dht22_temp_f = dht.readTemperature();     
  //Serial.println("DEBUG: [DHT22] Sensor Out H." + String((float)dht22_humidity) + "   T."  + String((float)dht22_temp_f));
  if (isnan(dht22_humidity) || isnan(dht22_temp_f)) {
    Serial.println("DEBUG: [DHT22] Failed to read from DHT sensor!");
    dht22_active=false;
  }else{
    dht22_active = true;
  }
}

boolean readSensors(){
  _readDht22Sensors();

  return dht22_active;
}

