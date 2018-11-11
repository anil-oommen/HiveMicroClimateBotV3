/*
 * Resistor , 500 - 3K Ohm
 * Used 3 x 9K Ressistor to get 3.3 K Ohm.
 */

#define LED_NOTIFY_AVALI "OOM Library Notify is available"

/* LED GREEN NOTIFY */
int GREEN_LED_PIN = D2;
int GREEN_ONOFF = 0;

/* LED RED NOTIFY */
int RED_LED_PIN = D1;
int RED_ONOFF = 0;

void setupLEDNotify(){
  pinMode(GREEN_LED_PIN, OUTPUT); 
  digitalWrite(GREEN_LED_PIN, 0);

  pinMode(RED_LED_PIN, OUTPUT); 
  digitalWrite(RED_LED_PIN, 0);
}


//Private Method, No need for external use.
void _ledBlink(int theLedPIN,int count, int theDelay){
  for(int i=0;i<count;i++){
    digitalWrite(theLedPIN, 1);
    delay(theDelay);
    digitalWrite(theLedPIN, 0);
    delay(theDelay);
  }
}


void greenLEDBlinkShort(int count){
  GREEN_ONOFF=0;
  _ledBlink(GREEN_LED_PIN,count, 100);
  GREEN_ONOFF=0;
}

void greenLEDBlinkLong(int count){
  GREEN_ONOFF=0;
  _ledBlink(GREEN_LED_PIN,count, 1000);
  GREEN_ONOFF=0;
}

void redLEDBlinkShort(int count){
  RED_ONOFF=0;
  _ledBlink(RED_LED_PIN,count, 100);
  RED_ONOFF=0;
}

void redLEDBlinkLong(int count){
  RED_ONOFF=0;
  _ledBlink(RED_LED_PIN,count, 1000);
  RED_ONOFF=0;
}

void doLEDDance(){
    greenLEDBlinkShort(2);
    redLEDBlinkShort(2);  
    greenLEDBlinkShort(2);
    redLEDBlinkShort(2);  
    greenLEDBlinkShort(2);
    redLEDBlinkShort(2);  
    greenLEDBlinkShort(2);
    redLEDBlinkShort(2);
}


