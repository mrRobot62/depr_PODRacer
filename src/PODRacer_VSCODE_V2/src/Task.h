/*




*/
#ifndef _TASK_H_
#define _TASK_H_



#include <Arduino.h>
#include "PODRacer.h"
#include "Receiver.h"
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
    virtual void update(uint8_t armed, uint8_t preventLogging = 1){};

    //virtual void update(TaskData *data, uint8_t preventLogging = 1);   // use with a data struct as return 
    uint8_t getBlinkPattern() {return this->_blink_pattern;};


    /* reset TaskData structure, set taskID again*/
    inline void resetTaskData() {
      memset (bbd, 0, sizeof(bbd));
      bbd->data.task_id = _id;  
      sprintf(bbd->data.fwversion, "%s", bb->FWVersion());
    }

    /* set TaskData updated flag */
    inline void setUpdated(bool updated=true) {
      bbd->data.updated = updated;
    }

    // used to set a global channel value 
    // e.g. used by main routine to set explizit one or more receiver channel values
    // to avoid a complete reference to receiver
    void setGlobalChannel(uint8_t id, uint16_t v) {
      id = constrain(id, 0, NUMBER_CHANNELS);
      globalBBD.data.ch[id] = v;
    };

    // return an explicit gobal channel value
    uint16_t getGlobalChannel(uint8_t id) {
      id = constrain(id, 0, NUMBER_CHANNELS);
      return globalBBD.data.ch[id];
    }

  protected:
    Receiver *recv;
    char *_tname;
    uint8_t _blink_pattern;
    uint8_t _id;
    TaskData globalBBD;

  protected:
    void setBlinkPattern(uint8_t pattern) {this->_blink_pattern = pattern;};

};
#endif