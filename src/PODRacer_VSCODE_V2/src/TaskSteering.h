/*




*/
#ifndef _TSTEERING_H_
#define _TSTEERING_H_

#include "Task.h"

class TaskSteering : public Task {
  public:
    TaskSteering(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema);

    void init(void) {;};      // implementation form abstract class
    void begin(bool allowLog = 0); 
    void update(bool armed, bool allowLog = 0){;};
    void update(TaskData *data, uint8_t preventLogging = 1);

  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode) {
      return td;
    }
};


#endif