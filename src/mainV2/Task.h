/*




*/
#ifndef _TASK_H_
#define _TASK_H_

#include <Arduino.h>
#include "PODRacer.h"
#include <SimpleKalmanFilter.h>

//------------------------------------------------------------------------
// Abstact class for all task
// This class inherits the PODRacer abstract class.
//------------------------------------------------------------------------

class Task : public PODRacer {
  public:
    Task(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema): PODRacer(log, name, taskSema) {
      this->_id = taskID;
      this->_tname = name;
    };
    // virtual ~Task();
    virtual void init(void) = 0;
    virtual void begin(uint8_t preventLogging = 1);
    virtual void update(uint8_t preventLogging = 1);
    virtual void update(TaskData *data, uint8_t preventLogging = 1);   // use with a data struct as return 
    uint8_t getBlinkPattern() {return this->_blink_pattern;};

  protected:
    char *_tname;
    uint8_t _blink_pattern;
    uint8_t _id;

  protected:
    void setBlinkPattern(uint8_t pattern) {this->_blink_pattern = pattern;};


};
#endif