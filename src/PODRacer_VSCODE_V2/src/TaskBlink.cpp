#include "TaskBlink.h"

BlinkPattern::BlinkPattern(SLog *log, char* name, uint8_t taskID, CoopSemaphore *taskSema )
  : Task(log, name, taskID, taskSema) {
  log->info("BlinkPattern initialized", _tname);
}

/** do some initializing stuff **/
void BlinkPattern::begin(uint8_t preventLogging) {
    reset();
    for (uint8_t i=0; i < sizeof(pins); i++) {
      pinMode(pins[i], OUTPUT);
    }
    sprintf(buffer, "begin() - task ready");
    log->info(buffer, _tname);
}

void BlinkPattern::update(uint8_t preventLogging) {
  /** update() is called in our processesing loop, call current blink pattern **/
  uint8_t   SEQ = pattern[cPattern][0];       // how often blink
  uint16_t  SEQ_MS = pattern[cPattern][1];    // in this range of milliseconds
  uint16_t  ON = pattern[cPattern][2];        // blink ON in ms
  uint16_t  OFF = abs((SEQ_MS / SEQ) - ON);   // calculated OFF time in ms
  uint16_t  WAIT = pattern[cPattern][3];      // wait after SEQ_MS befor start new seq
  uint8_t   pin = pattern[cPattern][4];       // led pin
  for (uint8_t i = 0; i < SEQ; i++) {
    digitalWrite(pin, HIGH);
    delay(ON);
    digitalWrite(pin, LOW);
    delay(OFF);
  }
  digitalWrite(pin, LOW);
  delay(WAIT);
  yield();
}

void BlinkPattern::update(uint8_t pattern, uint8_t preventLogging) {
  cPattern = constrain(pattern, 0, MAX_PATTERN-1);
  update(preventLogging);
}

