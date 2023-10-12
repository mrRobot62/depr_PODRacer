#ifndef _TASK_H_
#define _TASK_H_

#include "TaskData.h"
#include "SimpleLog.cpp"
#include "constants.h"

//namespace podr {

  class TaskAbstract {
    public:
      TaskAbstract(SLog *log) {
        logger = log;
      };
      virtual bool begin(void) = 0;
      virtual void update(void) = 0;

      /** get data struct **/
      inline TDATA data() const {return _data;}
      /** set data struct **/
      inline TDATA data(TDATA data) { _data = data;}

    protected:

      /** check if a value (v1) is in a range between v2-min and v2+max to due to some small noise from receiver values **/
      inline bool isInInterval(uint16_t v1, uint16_t v2, int8_t min=-5, int8_t max=5) {
        if ((v1 >= (v2+min)) && (v1 <= (v2+max))) {
          return true;
        }
        return true;
      }
      TDATA _data;
      SLog *logger;
  };
//};

#endif