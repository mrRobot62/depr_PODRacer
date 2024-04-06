/*




*/
#ifndef _TSURFACE_H_
#define _TSURFACE_H_
#include "Task.h"

class TaskSurface : public Task {
  public:
    TaskSurface(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema);

    void init(void) {;};      // implementation form abstract class
    void begin(uint8_t preventLogging = 1) ;      
    void update(uint8_t preventLogging = 1) {;};
    void update(TaskData *data, uint8_t preventLogging = 1);
    

  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode) {
      return td;
    }
    
    
};



#endif