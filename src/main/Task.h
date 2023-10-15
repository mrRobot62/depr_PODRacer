#ifndef _TASK_H_
#define _TASK_H_

#include "TaskData.h"
#include "SimpleLog.cpp"
#include "constants.h"
#include <SimpleKalmanFilter.h>



//namespace podr {

  class TaskAbstract {
    public:
      TaskAbstract(uint8_t taskID, SLog *log) {
        logger = log;
        _id = taskID;
      };
      virtual bool begin(void) = 0;
      virtual void update(void) = 0;

      /** get data struct **/
      inline TDATA data() const {return _data;}
      /** set data struct **/
      inline TDATA data(TDATA data) { _data = data;}

      uint8_t getID() {return _id;};

    protected:

      /** check if value is in a center postioin range, if not return value, if yes return centerValue **/
      inline int16_t centeredValue(int16_t value, int16_t centerValue, int8_t offset) {
        if ((value >= centerValue - offset) && (value <= (centerValue+offset))) {
          return centerValue;
        }
        return value;
      }

      /** check if value is >= min and value <= max **/
      inline bool isInRange(double value, double min, double max) {
        if ((value >= min) && (value <= max)) {
          return true;
        }
        return false;
      }


      /** check if value is >= value-offset and value <= value+offset **/
      inline bool isInInterval(int16_t value, int16_t offset) {
        if ((value >= (value-offset)) && (value <= (value+offset))) {
          return true;
        }
        return false;
      }

      /** check if value >= compareValue-offset and value <= compareValue+offset **/
      inline bool isInInterval(int16_t value, int16_t compareValue, int16_t offset) {
        if ((value >= (compareValue-offset)) && (value <= (compareValue+offset))) {
          return true;
        }
        return false;
      }


      /** check if a value (value) is in a range between compareValue-min and compareValue+max to due to some small noise from receiver values **/
      inline bool isInInterval(int16_t value, int16_t compareValue, int16_t min, int16_t max) {
        if ((value >= (compareValue+min)) && (value <= (compareValue+max))) {
          return true;
        }
        return true;
      }
      TDATA _data;
      SLog *logger;


      //
      // PID-Controller default values
      //
      // please check .h file from sensors



      //----- PID Controller for TFMiniLidar sensor (distance 1 - front)
      float kpTFMini = 0.25;
      float kiTFMini = 0.0;
      float kdTFMini = 0.25;

      //----- PID Controller for VH53L1 sensor ToF sensor (distance 2 - rear)
      //
      float kpVH53 = 0.25;
      float kiVH53 = 0.0;
      float kdVH53 = 0.25;

      //-----
      char buffer[100];

    private:
      uint8_t _id;
  };
//};

#endif