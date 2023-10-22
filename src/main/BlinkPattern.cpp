#include "BlinkPattern.h"


  BlinkPattern::BlinkPattern(uint8_t taskID, SLog *log) : TaskAbstract(taskID, log)  {
    logger->info("BlinkPattern initialized");
  }

  bool BlinkPattern::begin(uint8_t *blink_pattern) {
    cPattern = 0;
    resetError();
    logger->info("BlinkPattern ready");
    return true;
  }

  void BlinkPattern::update(void) {
    uint8_t pin = pattern[cPattern][3];
    uint8_t pos = 0;
    for (uint8_t b=0; b < 8; b++) {
      if (pattern[cPattern][0] & idx[b]) {
        digitalWrite(pin, HIGH);
        delay(pattern[cPattern][1]);
        digitalWrite(pin, LOW);
        delay(100);
      }
      else {
        digitalWrite(pin, LOW);
        delay(pattern[cPattern][2]);
      }
    }
    digitalWrite(pin, LOW);
    /*
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(pattern[cPattern][0]);
    digitalWrite(pin, LOW);
    delay(pattern[cPattern][1]);
    */
  }

  void BlinkPattern::update(uint8_t id) {
    cPattern = constrain(id, 0, MAX_PATTERN-1);
    //cPattern = 0; // can be used to test new patterns ;-)
    update();
  }


