/*




*/
#ifndef _TCLI_H_
#define _TCLI_H_
#include "Task.h"

class TaskCLI : public Task {
  public:
    TaskCLI(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema);

    void init(void) {;};      // implementation form abstract class
    void begin(bool allowLog = 0) ;        
    void update(bool armed, bool allowLog = 0);

    TaskData *getTaskData() {return bbd;};

  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode) {
      return td;
    }

    private:
        bool cli_enabled = false;
};
#endif