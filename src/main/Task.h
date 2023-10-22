#ifndef _TASK_H_
#define _TASK_H_

#include "TaskData.h"
#include "SimpleLog.cpp"
#include "constants.h"
#include <SimpleKalmanFilter.h>
#include <CoopSemaphore.h>



//namespace podr {



  class TaskAbstract {
    public:
//      TaskAbstract(uint8_t taskID, SLog *log, CoopSemaphore *sema) {
      TaskAbstract(uint8_t taskID, SLog *log, CoopSemaphore *sema=nullptr) {
        logger = log;
        _id = taskID;
        _sema = sema;
      };
      virtual bool begin(void) = 0;
      virtual void update(void) = 0;

      inline const bool hasError(void) {
        return (errorCode > 0)?true:false;
      }

      inline const uint8_t getError(void) {
        return errorCode;
      }

      inline void resetError() {
        errorCode = 0;
      }

      /** get data struct **/
      inline TDATA data() const {return _data;}
      /** set data struct **/
      inline TDATA data(TDATA data) { _data = data;}

      inline uint8_t getID() {return _id;};

      inline bool isUpdated() {
        return _data.updated;
      }

      inline void setUpdateFlag() {
        _data.updated=true;
      }

      inline void resetUpdateFlag() {
        _data.updated=false;
      }
      

    protected:

      inline void setError(uint8_t code) {
        errorCode = code;
        resetUpdateFlag();
      }

      /** check if value is in a center postioin range, if not return value, if yes return centerValue **/
      inline int16_t centeredValue(int16_t value, int16_t centerValue, int8_t offset) {
        if ((value >= (centerValue - offset)) && (value <= (centerValue+offset))) {
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
      CoopSemaphore *_sema;

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
      uint8_t _blink_pattern;
    private:
      uint8_t _id;
      uint8_t errorCode;
  };
//};

#endif