#ifndef _BlinkPattern_H_
#define _BlinkPattern_H_

#include <Arduino.h>
#include "Task.h"
#include "constants.h"

#define MAX_PATTERN 10

class BlinkPattern : public TaskAbstract {
  public:
    BlinkPattern(uint8_t taskID, SLog *log, Blackbox *bb=nullptr);

    /** initialize **/
    bool begin(void) {;};
    bool begin(uint8_t *blink_pattern);
    /** update site loop **/
    void update(void);
    void update(uint8_t pattern) ;

    inline void setPattern(uint8_t pattern) {
      cPattern = constrain(pattern, 0, MAX_PATTERN-1 );
    }

  private:
    /**
      0b00000000 = Bit pattern 1=ON, 0=OFF (from right to left)
      ON-TIME in ms
      OFF-time in ms
      PIN

      {0b00001010, 500, 100, 2}
      0b00001010    OFF   ON   OFF  ON  OFF OFF OFF OFF
                    100   500  100  500 100 100 100 100
      LED on pin 2 (build-in led)
      
      Note: if ON an internal 100ms delay is done
    **/
    uint8_t cPattern, defaultPattern;
    uint8_t idx[8] = {1,2,4,8,16,32,64,128};
    uint8_t pins[2] = {2,15};
    uint16_t pattern[MAX_PATTERN][4] = {
      {0b00000001, 500, 200, 15}, // 0 TASK_HB (blink 1x in 2secs (7xoff with 200ms =1400ms + 1xON with 500ms = 1900ms +100ms(internal) = 2000ms blink))
      {0b00010001, 100, 200, 15}, // 1 TASK_HOVER
      {0b00001011, 500, 200, 2}, // 2 TASK_OPTICALFLOW
      {0b00000000, 250, 250, 2}, // 3 TASK_SURFACEDISTANCE
      {0b00000000, 250, 250, 2}, // 4 TASK_FRONTDISTANCE
      {0b01010101, 100,  50, 2}, // 5 TASK_RECEIVER
      {0b00000000, 250, 250, 2}, // 6 TASK_ARBITRATE
      {0b00000000, 250, 250, 2}, // 7 free
      {0b00000000, 250, 250, 2}, // 8 free
      {0b00000000, 250, 250, 2}  // 9 free
    };
 
};

#endif