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
    Task(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema): PODRacer(log, name, taskSema) {
      this->bbd = nullptr;
      this->_id = taskID;
      this->_tname = name;
      resetTaskData(true, true);
      sprintf(buffer, "TaskID:%d at %d, TaskData: %d",
         this->_id, this, this->bbd
      );
      log->info(buffer, true, name);

      this->setInternalError(taskID,0);
      for (uint8_t i=0; i < 5; i++) {
        ignore_sensor[i] = false;
      }
      log->debug("Task constructor loaded", true, name);
    };
    // virtual ~Task();
    virtual void init(void) = 0;
    virtual void begin(bool allowLog = 0) = 0;
    virtual void update(bool armed, bool allowLog = 0) = 0;

    //virtual void update(TaskData *data, uint8_t preventLogging = 1);   // use with a data struct as return 
    uint8_t getBlinkPattern() {return this->_blink_pattern;};


    /* reset TaskData structure, set taskID again*/
    inline void resetTaskData(bool createNew=false, bool allowLogging=false) {
      if (createNew) {
        if (this->bbd != nullptr) {
          //sprintf(buffer,"free this->bbd at %d", this->bbd);
          //log->info(buffer, true, this->name);
          free(this->bbd);
        }
        this->bbd = new TaskData();
        log->info("create new TaskData-Object", allowLogging, name);
      }
      else {
        memset (bbd, 0, sizeof(bbd));
      }

      for (uint8_t ch=0;ch < NUMBER_CHANNELS; ch++) {
        bbd->data.ch[ch] = 0;
      }
      bbd->data.task_id = this->_id;  
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
      this->internal_error_occured = (code > 0)? true: false;
      this->internal_error_code = (taskID << 8 ) + code;
    }

  protected:
    Receiver *recv;
    const char*_tname;
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