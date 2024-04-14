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
      this->setInternalError(0,0);
      for (uint8_t i=0; i < 5; i++) {
        ignore_sensor[i] = false;
      }
    };
    // virtual ~Task();
    virtual void init(void) = 0;
    virtual void begin(bool allowLog = 0);
    virtual void update(uint8_t armed, bool allowLog = 0){};

    //virtual void update(TaskData *data, uint8_t preventLogging = 1);   // use with a data struct as return 
    uint8_t getBlinkPattern() {return this->_blink_pattern;};


    /* reset TaskData structure, set taskID again*/
    inline void resetTaskData() {
      memset (bbd, 0, sizeof(bbd));
      // sprintf(buffer, "delete bbd(%d)", bbd);
      // log->info(buffer, true, name);
      // delete bbd;
      // bbd = new TaskData();
      // sprintf(buffer, "create new bbd(%d)", bbd);
      // log->info(buffer, true, name);
      for (uint8_t ch=0;ch < NUMBER_CHANNELS; ch++) {
        bbd->data.ch[ch] = 0;
      }
      bbd->data.task_id = _id;  
      sprintf(bbd->data.fwversion, "%s", bb->FWVersion());
      setInternalError(this->_id, 0);
      setUpdated(false);
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

    // return an explicit gobal channel value, this value is set globally in for(;;)-task loops
    uint16_t getGlobalChannel(uint8_t id) {
      id = constrain(id, 0, NUMBER_CHANNELS);
      return globalBBD.data.ch[id];
    }

    bool isInternalError() {
      return internal_error_occured;
    }
    uint16_t getInternalErrorCode() {
      return internal_error_code;
    }

    /* set error code and internal flag, of code=0, reset error code and flag*/
    void setInternalError(uint8_t taskID, uint8_t code) {
      this->internal_error_code = (taskID << 8 ) + code;
      this->internal_error_occured = (code > 0)? true: false;

    }

  protected:
    Receiver *recv;
    char *_tname;
    uint8_t _blink_pattern;
    uint8_t _id;
    TaskData globalBBD;
    bool internal_error_occured;
    uint16_t internal_error_code;
    bool ignore_sensor[5];          // can be used by Task to ignore a sensor (normally used for development)

  protected:
    void setBlinkPattern(uint8_t pattern) {this->_blink_pattern = pattern;};

};
#endif