/*




*/
#ifndef _TBLINK_H_
#define _TBLINK_H_

#include "Task.h"

#define MAX_PATTERN 10



class BlinkPattern : public Task {
  public:
    BlinkPattern(SLog *log, char* name, uint8_t taskID, CoopSemaphore *taskSema);
    // virtual ~BlinkPattern(){};
    void init(void) {;};      // implementation form abstract class
    void begin(bool allowLog = 0);     // implementation form abstract class
    void update(bool allowLog = 0);
    void update(uint8_t pattern, bool allowLog = 0);
    void update(TaskData *data, bool allowLog = 0){;};
    TaskData *getMockedData(TaskData *td, uint8_t mode){;};

    inline void reset(void) {
      /* reset current Pattern to standard (0) */
      cPattern = 0;
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
    // uint8_t idx[8] = {1,2,4,8,16,32,64,128};
    uint8_t pins[2] = {2,15};
    uint16_t pattern[MAX_PATTERN][5] = {
        // <number of blinks>, <in seq-ms>, <ms on>, <wait ms after seq>, <pin>
        // 8, 50, 1000, 15 = 8 flashes per sec (), ON=50ms, OFF=is calculated wait 1000ms on pin 15
        // OFF time calculation = 1000 / <number of blinks) = sequence_ms per blink - ON = OFF-in_ms
        //  1000 / 8 = 125 - 50 = 75ms OFF
        {1, 2000, 500,  0, 15},     // 0 = HeartBeat, (1x per 2000ms, ON=500ms, OFF=1500ms, wait=0ms, repeat)
        {1, 1000, 50,   1000, 15},  // 1 =
        {1, 1000, 50,   1000, 15},  // 2 =
        {1, 1000, 50,   1000, 15},  // 3 =
        {1, 1000, 50,   1000, 15},  // 4 =
        {1, 1000, 50,   1000, 15},  // 5 = 
        {1, 1000, 500,   500, 15},  // 6 = ARMED
        {1, 1000, 50,   1000, 15},  // 7 = EMERGENCY ()
        {4, 1000, 100,  200,  15},  // 8 = DISARMED ( 4x per 1000ms, ON=100ms OFF = 150ms, wait=200)
        {8, 1000, 25,   0,  15},    // 9 = PREVENT_ARMING (8 flashes per sec, 25ms ON, 100ms OFF, wait 0ms, repeat)

      };
};


#endif