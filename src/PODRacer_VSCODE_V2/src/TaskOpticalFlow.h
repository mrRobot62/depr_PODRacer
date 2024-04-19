/*




*/
#ifndef _TOFLOW_H_
#define _TOFLOW_H_
#include "Task.h"

class TaskOpticalFlow : public Task {
  public:
    TaskOpticalFlow(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema);

    void init(void) {;};      // implementation form abstract class
    void begin(bool allowLog = 1) ;      
    void update(bool allowLog = 1) {;};
    void update(bool armed, bool allowLog = 0){;};
    void update(TaskData *data, bool allowLog = 1);

  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode) {
      return td;
    }

};




#endif