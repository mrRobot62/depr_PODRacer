/*

*/
#ifndef _PODRACER_H_
#define _PODRACER_H_
#include <CoopTaskBase.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <FastCRC.h>
#include "TaskData.h"
#include "Blackbox.h"
#include "Logger.h"
#include "global.h"
#include "global_utils.h"
//#include "globalvars.h"


//------------------------------------------------------------------------
// Main Abstact class for everything
// All classes which are not Tasks inherit from this base class
// The base task class inherit from this class too
//
//------------------------------------------------------------------------

class PODRacer {
  public:
    
    /* PODRacer constructur SLog = Logger, char* name = Name of this object, semaphore is default null, mock (only for test purpose) */
    PODRacer(SLog *log, const char *name, CoopSemaphore *taskSema = nullptr) {
      //Serial.print("constructor PODRacer "); Serial.println(name);
      this->log = log;
      this->name = name;
      this->semaphore = taskSema;
      this->bb = new Blackbox(log, BLACKBOX_CS_PIN);
      // mock can be used in tasks, if > 0, than data is not read sensor data, its generated (hardcoded) testdata
      // the value of mock depends on the task and what kind of test data should be used.
      // this is only used during development !!!!!!
      this->mock_level = 0;
      sprintf(buffer,"Constructor PODRacer::%s created...", name);
      log->info(buffer, true, name);
      log_once_mask = 0;
    }
    // ~PODRacer(){;};
    virtual TaskData *getMockedData(TaskData *td, uint8_t mode) = 0;
    TaskData *getTaskData(){return bbd;};

    // set a mock level. Mock leves can be used in tasks to generate different data for a task (like different receiver-read data)
    void setMock(uint8_t mock_level) {this->mock_level = mock_level;};
    uint8_t getMockLevel() {return this->mock_level;};
    // reset the mock level - mocking is deactivated with a mock=0 (in production !)
    void resetMockLevel() {this->mock_level = 0;}
    void resetLogOnceBit(uint16_t *once_mask, uint8_t maskBit) {
      (maskBit > 15)?maskBit=15:maskBit;
      bitSet(*once_mask, maskBit);
    }

  protected:
    char buffer[300];
    bool blocked;                       // used as return value for Semaphore.wait() result. If true, blocked by an other task. Acquaring task has to wait

    const char* name;
    //uint8_t log_once[LOG_ONCE_IDX]; // ten differen types of messages can be used, to print only once per run (typically used by Disarming, Arming, .... messages)
    uint16_t log_once_mask;         // every class can be used 16 different bit position to work with logs which should be printed only once
    SLog *log;
    CoopSemaphore *semaphore;
    Blackbox *bb;                   // Blackbox object
    TaskData *bbd;                  // DataStruct (blackbox-data)
    uint8_t mock_level;             // store an ID for mock-data, if 0 no mocking data is available (production!)

};


#endif