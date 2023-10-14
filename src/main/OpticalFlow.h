#ifndef _OPTICAL_FLOW_H_
#define _OPTICAL_FLOW_H_

#include <Arduino.h>
#include "SimpleLog.cpp"
//#include "TaskData.h"
#include "Task.h"
#include <Bitcraze_PMW3901.h>
#include <PID_v1.h>
#include "constants.h"

  /**
    Use a PMW3901 optical flow sensor

    This sensor is used to check if PODRacer is on a movement over ground in +/-X and +/-Y direction

    -X+Y. |.  +X+Y
    ------|-------
    -X-Y. |.  +X-Y

  **/
  class OpticalFlow : public TaskAbstract {
    public:
      OpticalFlow(SLog *log, uint8_t cs_pin);
      bool begin(void);
      void update(void);

    private:
      //TDATA data;
      Bitcraze_PMW3901 *flow;
      uint8_t _cs;
      
      // rawX/Y store the raw sensor values for X/Y
      // we add this values to the slipping X/Y values
      int16_t rawX, rawY;
     
      // the slipping value for X and Y direction
      double slipX, slipY;

      // the adjusted slipping value for X/Y
      double slipAdjX, slipAdjY;

      // the PID controllyer should try to get this setpoint
      double setPointSlipX, setPointSlipY;

      //----- PID Controller for OpticalFlow sensor
      double kpOpticalFlow = 0.25;
      double kiOpticalFlow = 0.0;
      double kdOpticalFlow = 0.25;

      PID *pidX, *pidY;
  };

#endif
