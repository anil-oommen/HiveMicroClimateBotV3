
/* Banner Generator http://patorjk.com/software/taag , Use 'Digital' */


String prntBanner_HiveBotStartup() {
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
Serial.println(" |H|I|V|E|B|O|T| | | | | | | | | | | | | | | | | | | | | | | | | | | |");
Serial.print(" ");
Serial.print(bot_id);
Serial.print(" (");
Serial.print(bot_version);
Serial.print(")");
Serial.println( bot_desc );
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
}

String prntBanner_InstructionExecute() {
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
Serial.println(" |I|n|s|t|r|u|c|t|i|o|n| |E|x|e|c|u|t|e| |S|t|a|r|t| |-|-|-|-|-|-|-|-|");
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+ +-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
}

String prntBanner_InstructionComplete() {
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
Serial.println(" |I|n|s|t|r|u|c|t|i|o|n| |C|o|m|p|l|e|t|e|d| |-|-|-|-|-|-|-|-|");
Serial.println(" +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+");
}


/*
 * EventTimer allow you schedule Event recuring for a specified time.
 */

class EventTimer
{
  private:
    String _timerName = "";
    unsigned long _lastEventAtMs=0;
    int _evenFreqMilliSecs =1000;
    long counter=0;
    boolean _isenabled = false;
    boolean _run_from_bootup_or_last_run_if_connected_or_enabled = false;
  public:
    EventTimer(String timerName, long freqMillSecs, boolean is_enabled, boolean runFromBootupOrLastRunIfConnectedOrEnabled);
    boolean isDueForRun();
    long runCounts();
    boolean enabled(boolean is_enabled);
    int runFrequency();
};

EventTimer::EventTimer(String timerName, long freqMillSecs,boolean is_enabled, boolean runFromBootupOrLastRunIfConnectedOrEnabled){
  this->_evenFreqMilliSecs=freqMillSecs;
  this->_timerName = timerName;
  this->_run_from_bootup_or_last_run_if_connected_or_enabled = runFromBootupOrLastRunIfConnectedOrEnabled;
  this->enabled(is_enabled);
}
long EventTimer::runCounts(){
  return this->counter;
}
int EventTimer::runFrequency(){
  return this->_evenFreqMilliSecs;
}
boolean EventTimer::enabled(boolean is_enabled){
  this->_isenabled=is_enabled;
  if(this->_isenabled){  
    if(!this->_run_from_bootup_or_last_run_if_connected_or_enabled){
      //Dont Run Immediate, Reset Timer from Now.
      unsigned long currenttMS =millis();
      this->_lastEventAtMs =currenttMS; //Note at BootTime
    }else{
      // Note, its not immediate, timer is still from bootup time
    }
  }
  return this->_isenabled;
}
boolean EventTimer::isDueForRun(){
  if(!this->_isenabled) return false;
  unsigned long currenttMS =millis();
  signed long timeRemaining =  (this->_lastEventAtMs + this->_evenFreqMilliSecs ) - currenttMS;
  /* Enable for Debugging 
  if(this->_timerName.indexOf("#")<0){ //Debug Information if it does not have #
    Serial.print("DEBUG: [EVENT_TIMER] (ms)" );
    Serial.print(this->_timerName);
    Serial.print(" TimeRemaining(s ");
    Serial.print(String(timeRemaining/1000));
    Serial.print("), LastRunAt( ");
    Serial.print(String((this->_lastEventAtMs)));
    Serial.print("), CurrTime( ");
    Serial.print(String(currenttMS));
    Serial.print("), interval(s  ");
    Serial.print(String(this->_evenFreqMilliSecs/1000));
    Serial.println(")");
  }
  */
  if (timeRemaining<=0){
    this->counter++;
    this->_lastEventAtMs =currenttMS;
    return true;
  }else{
    return false;
  }
}

/* Piezo Beepers */
#define PIEZO_TRANSDUCER_PIN D5  //cet12a35

void _beepRaw(unsigned char delayms, unsigned int dutyCycle){
  analogWrite(PIEZO_TRANSDUCER_PIN, dutyCycle);
  delay(delayms);
  analogWrite(PIEZO_TRANSDUCER_PIN, 0);
  delay(delayms);
}  
void beepAcknowledge(){_beepRaw(50,132);}
void beepError(){_beepRaw(50,132);_beepRaw(50,132);_beepRaw(50,132);}


