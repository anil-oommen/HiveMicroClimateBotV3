/*
 * IRremoteESP8266: IRrecvDumpV2 - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 *
 * Copyright 2009 Ken Shirriff, http://arcfn.com
 * Copyright 2017 David Conran
 *
 * Example circuit diagram:
 *  https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
 *
 * Changes:
 *   Version 0.3 November, 2017
 *     - Support for A/C decoding for some protcols.
 *   Version 0.2 April, 2017
 *     - Decode from a copy of the data so we can start capturing faster thus
 *       reduce the likelihood of miscaptures.
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
 */

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <IRsend.h>

#if DECODE_AC
#include <ir_Daikin.h>
#include <ir_Fujitsu.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Toshiba.h>
#endif  // DECODE_AC

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
#define IR_RECV_PIN D6 //8  //old 14
#define IR_SEND_PIN D8

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define CAPTURE_BUFFER_SIZE 1024

// TIMEOUT is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best TIMEOUT value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed MAX_TIMEOUT_MS. Typically 130ms.
#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
                     // e.g. Kelvinator
                     // A value this large may swallow repeats of some protocols
#else  // DECODE_AC
#define TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
#endif  // DECODE_AC
// Alternatives:
// #define TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
                        // units, but can accidentally swallow repeated messages
                        // in the rawData[] output.
// #define TIMEOUT MAX_TIMEOUT_MS  // This will set it to our currently allowed
                                   // maximum. Values this high are problematic
                                   // because it is roughly the typical boundary
                                   // where most messages repeat.
                                   // e.g. It will stop decoding a message and
                                   //   start sending it to serial at precisely
                                   //   the time when the next message is likely
                                   //   to be transmitted, and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the TIMEOUT value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
#define MIN_UNKNOWN_SIZE 12
// ==================== end of TUNEABLE PARAMETERS ====================


// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(IR_RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);

decode_results results;  // Somewhere to store the results

// Display the human readable state of an A/C message if we can.
String describeACInfo(decode_results *results) {
  String description = "";
  String decode_type_desc ="UNKNOWN";
#if DECODE_DAIKIN
  if (results->decode_type == DAIKIN) {
    IRDaikinESP ac(0);
    ac.setRaw(results->state);
    description = ac.toString();
    decode_type_desc= "DAIKIN";
  }
#endif  // DECODE_DAIKIN
#if DECODE_FUJITSU_AC
  if (results->decode_type == FUJITSU_AC) {
    IRFujitsuAC ac(0);
    ac.setRaw(results->state, results->bits / 8);
    description = ac.toString();
    decode_type_desc= "FUJITSU_AC";
  }
#endif  // DECODE_FUJITSU_AC
#if DECODE_KELVINATOR
  if (results->decode_type == KELVINATOR) {
    IRKelvinatorAC ac(0);
    ac.setRaw(results->state);
    description = ac.toString();
    decode_type_desc= "KELVINATOR";
  }
#endif  // DECODE_KELVINATOR
#if DECODE_TOSHIBA_AC
  if (results->decode_type == TOSHIBA_AC) {
    IRToshibaAC ac(0);
    ac.setRaw(results->state);
    description = ac.toString();
    decode_type_desc= "TOSHIBA_AC";
  }
#endif  // DECODE_TOSHIBA_AC
#if DECODE_MIDEA
  if (results->decode_type == MIDEA) {
    IRMideaAC ac(0);
    ac.setRaw(results->value);  // Midea uses value instead of state.
    description = ac.toString();
    decode_type_desc= "MIDEA";
  }
#endif  // DECODE_MIDEA

  // If we got a human-readable description of the message, display it.
  if (description != ""){
    return "IRDecodeType:" + decode_type_desc + ",Source: IRSensor," +description;
    //Serial.println("Mesg Desc.: " + description);
  }else{
    return "IRDecodeType:UNKNOWN";
  }
}

IRKelvinatorAC kelvir(IR_SEND_PIN);  // An IR LED is controlled by GPIO4, NodeMCU D2
void setupIRModule(){
  #if DECODE_HASH
      // Ignore messages with less than minimum on or off pulses.
      irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
  #endif  // DECODE_HAS
  irrecv.enableIRIn(); 
  //irsend.begin();

  //Initilize IR Settings for Kelvinator
  kelvir.begin();

}

/*
 * mode : KELVINATOR_AUTO, KELVINATOR_COOL, KELVINATOR_HEAT, KELVINATOR_DRY, KELVINATOR_FAN
 */
int _acProfileId = -1;
String getAirconfProfileDataMap(){
  String dataMap =  "\"AcPower\": \"";
  if(kelvir.getPower()) dataMap +="ON\""  ;
  else dataMap +="OFF\""  ;
  dataMap += ",\"AcTemp\": \""+ String(kelvir.getTemp()) +"\""  ;
  dataMap += ",\"AcMode\": \""+ String(kelvir.getMode()) +"\""  ;
  dataMap += ",\"AcFan\": \""+ String(kelvir.getFan()) +"\""  ;
  dataMap += ",\"AcProfileId\": \""+ String(_acProfileId) +"\""  ;
  return dataMap;
}
void setAirconProfile(int acProfileId ,boolean on_off, int temp, uint64 mode, uint8_t fanSpeed){
  /*
   * Set Default 
   */
  _acProfileId  = acProfileId;

  if(!on_off){
    kelvir.setFan(0);
    kelvir.setLight(true);
    kelvir.setMode(KELVINATOR_AUTO);
    kelvir.off();
  }else{
    kelvir.on();
    kelvir.setFan(fanSpeed);
    kelvir.setMode(mode);
    kelvir.setTemp(temp);
    kelvir.setSwingVertical(false);
    kelvir.setSwingHorizontal(true);
    kelvir.setXFan(true);
    kelvir.setIonFilter(false);
    kelvir.setLight(true);
  }

  
}

void sendAirconProfile(){
  kelvir.send();
}


unsigned int irContinusRunForSecs = 10;
unsigned long irLastInerrupttMS =0;
unsigned long irForceInterruptAtMS =0;
String irDataPayload = "";
//String irTolerentEncodedDataPayload = "";
boolean checkIRAndInteruptForOtherProcessing(){
  unsigned long irCurrenttMS =millis();
  if(
    irCurrenttMS > (irLastInerrupttMS + (irContinusRunForSecs * 1000) ) //Organic end of Exclusive Period
    || (irForceInterruptAtMS>1 && irCurrenttMS > irForceInterruptAtMS  ) //Force Interrupt after x Seconds already decided 
  ){
      //Interrupt
      irLastInerrupttMS = irCurrenttMS; 
      irForceInterruptAtMS=0;
      Serial.println("DEBUG: [IR_RECIEVE] Interrupting for IR Yield" );
      return true; //ok breakup, you have either data or time to yield for other work.
  }else{
    if (irrecv.decode(&results)) {
              // Display a crude timestamp.
      {    
            //Decode Section from original Codes. 
            
            uint32_t now = millis();
            Serial.printf("Timestamp : %06u.%03u\n", now / 1000, now % 1000);
            if (results.overflow)
              Serial.printf("WARNING: IR code is too big for buffer (>= %d). "
                            "This result shouldn't be trusted until this is resolved. "
                            "Edit & increase CAPTURE_BUFFER_SIZE.\n",
                            CAPTURE_BUFFER_SIZE);
            // Display the basic output of what we found.
            Serial.print(resultToHumanReadableBasic(&results));
            Serial.println(describeACInfo(&results));  // Display any extra A/C info if we have it.
            yield();  // Feed the WDT as the text output can take a while to print.
        
            // Display the library version the message was captured with.
            Serial.print("Library   : v");
            Serial.println(_IRREMOTEESP8266_VERSION_);
            Serial.println();
        
            // Output RAW timing info of the result.
            Serial.println(resultToTimingInfo(&results));
            yield();  // Feed the WDT (again)
        
            // Output the results as source code
            Serial.println(resultToSourceCode(&results));
            Serial.println("");  // Blank line between entries
            yield();  // Feed the WDT (again)
      }        
              
      Serial.print("DEBUG: [IRREAD] Readable:" );
      
      
      //I have IR Data.
      irDataPayload += describeACInfo(&results);
      irDataPayload += " ";
      Serial.println(irDataPayload);
      
      //String tolIRValues = _IRREMOTEESP8266_VERSION_;
      //irTolerentEncodedDataPayload +=tolIRValues;

      //We have IR Signal so ForceInterrupt in 2 Sec Later, and keep continuing to gather the reset of data.
      irForceInterruptAtMS = irCurrenttMS + (1 * 1000);
    }else{
      //No Data , lets Yield.
      yield();
    }
  }
  return false;//keep continuing to give me control to run.
}


// The repeating section of the code
//
/*
void loop() {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf("Timestamp : %06u.%03u\n", now / 1000, now % 1000);
    if (results.overflow)
      Serial.printf("WARNING: IR code is too big for buffer (>= %d). "
                    "This result shouldn't be trusted until this is resolved. "
                    "Edit & increase CAPTURE_BUFFER_SIZE.\n",
                    CAPTURE_BUFFER_SIZE);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    dumpACInfo(&results);  // Display any extra A/C info if we have it.
    yield();  // Feed the WDT as the text output can take a while to print.

    // Display the library version the message was captured with.
    Serial.print("Library   : v");
    Serial.println(_IRREMOTEESP8266_VERSION_);
    Serial.println();

    // Output RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)

    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println("");  // Blank line between entries
    yield();  // Feed the WDT (again)
  }
}
*/
