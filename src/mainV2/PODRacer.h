/*




*/
#ifndef _PODRACER_H_
#define _PODRACER_H_
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <FastCRC.h>
#include "TaskData.h"
#include "Blackbox.h"
#include "Logger.h"
#include "global.h"
#include "global_utils.h"

//------------------------------------------------------------------------
// Main Abstact class for everything
// All classes which are not Tasks inherit from this base class
// The base task class inherit from this class too
//
//------------------------------------------------------------------------

class PODRacer {
  public:
    
    /* PODRacer constructur SLog = Logger, char* name = Name of this object, semaphore is default null, mock (only for test purpose) */
    PODRacer(SLog *log, char* name, CoopSemaphore *taskSema = nullptr) {
      //Serial.print("constructor PODRacer "); Serial.println(name);
      this->log = log;
      this->name = name;
      this->semaphore = taskSema;
      this->bb = new Blackbox(log, BLACKBOX_CS_PIN);
      // mock can be used in tasks, if > 0, than data is not read sensor data, its generated (hardcoded) testdata
      // the value of mock depends on the task and what kind of test data should be used.
      // this is only used during development !!!!!!
      this->mock_level = 0;
      sprintf(buffer,"Constructor PODRacer::%s", name);
      log->info(buffer, name);
      for (uint8_t i = 0; i < LOG_ONCE_IDX; i++) {
        this->log_once[i] = 0;
      }
    }
    // ~PODRacer(){;};
    virtual TaskData *getMockedData(TaskData *td, uint8_t mode) {};
    virtual TaskData *getTaskData(){};

    // set a mock level. Mock leves can be used in tasks to generate different data for a task (like different receiver-read data)
    void setMock(uint8_t mock_level) {this->mock_level = mock_level;};
    uint8_t getMockLevel() {return this->mock_level;};
    // reset the mock level - mocking is deactivated with a mock=0 (in production !)
    void resetMock() {this->mock_level = 0;}

  protected:
    // create a dump from internal TaskData structure and ouput this on serial 
    void DumpTaskData(bool channels=true, bool flags=false, bool ldata=false, bool fdata=false, bool pid_rpyth=false, bool const_hover=false) {
      if (bbd == nullptr) return;
      if (channels) {
        sprintf(buffer, "DUMP::CHANNELS: CH[R]:%d, CH[P]:%d, CH[Y]:%d, CH[T]:%d, CH[H]:%d, CH[ARM]:%d", 
            bbd->data.ch[ROLL], bbd->data.ch[PITCH], bbd->data.ch[YAW], bbd->data.ch[THRUST], bbd->data.ch[HOVERING], bbd->data.ch[ARMING]);
        if (this->log_once[LOG_ONCE_IDX-3]==0) {log->info(buffer, name); this->log_once[LOG_ONCE_IDX-3]=1;}
      }
      if (flags) {
        sprintf(buffer, "DUMP::FLAGS: TaskID:%d, updated:%B, failsafe:%B, lost_frame:%b, ARMED, GrpA:%d, GrpB", 
            bbd->data.task_id, bbd->data.updated, bbd->data.failsafe, bbd->data.lost_frame, bbd->data.is_armed, bbd->data.groupA, bbd->data.groupB);
        if (this->log_once[LOG_ONCE_IDX-3]==0) {log->info(buffer, name); this->log_once[LOG_ONCE_IDX-3]=1;}
      }
    }


  protected:
    char buffer[300];
    bool blocked;                       // used as return value for Semaphore.wait() result. If true, blocked by an other task. Acquaring task has to wait

    char* name;
    uint8_t log_once[LOG_ONCE_IDX];     // ten differen types of messages can be used, to print only once per run (typically used by Disarming, Arming, .... messages)
    SLog *log;
    CoopSemaphore *semaphore;
    Blackbox *bb;                   // Blackbox object
    TaskData *bbd;                  // DataStruct (blackbox-data)
    bool mock_level;                      // can used to simulate data without a well runing task/object


};


#endif