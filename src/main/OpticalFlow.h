#ifndef _OPTICAL_FLOW_H_
#define _OPTICAL_FLOW_H_

#include <Arduino.h>
#include "SimpleLog.cpp"
//#include "TaskData.h"
#include "Task.h"
#include <Bitcraze_PMW3901.h>
#include "constants.h"

//namespace podr {

  /**
    Use a PMW3901 optical flow sensor

    This sensor is used to check if PODRacer is on a movement over ground in +/-X and +/-Y direction

    -X+Y. |.  +X+Y
    ------|-------
    -X-Y. |.  +X-Y

  **/
  class OpticalFlow : public TaskAbstract {
    public:
      OpticalFlow(SLog *log, uint8_t pin);
      bool begin(void);
      void update(void);

    private:
      //TDATA data;
      Bitcraze_PMW3901 *flow;
      uint8_t _pin;
      int16_t deltaX, deltaY;
  };
//};

#endif
