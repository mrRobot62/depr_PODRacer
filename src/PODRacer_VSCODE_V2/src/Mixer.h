/*




*/
#ifndef _MIXER_H_
#define _MIXER_H_
#include "PODRacer.h"
#include "Task.h"

#define TASK_LIST_SIZE 10


class Mixer : public PODRacer {
  public:
    Mixer(SLog *log, char *name, CoopSemaphore *taskSema = nullptr)
      : PODRacer(log, name, taskSema) {
      for (uint8_t t=0; t < TASK_LIST_SIZE; t++) {
        taskList[t].taskID = -1;
        taskList[t].task = nullptr;
      }
    };
    // ~Mixer() {};
    void init() {;};
    void begin(bool allowLog = 0);
    void addTask(Task  *task, uint8_t taskID){
      uint8_t id = constrain(number_of_tasks++, 0, TASK_LIST_SIZE);
      taskList[id].task = task;
      taskList[id].taskID = taskID;
    } ;
  
    void update(TaskData *data, bool allowLog = 0); 
    
    TaskData *getTaskData() {return this->tdw;};
  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode);

  protected:
    TaskData *tdr = nullptr;
    TaskData *tdw = nullptr;

    typedef struct {
      int taskID;
      Task *task;
    } TaskList;

    TaskList taskList[TASK_LIST_SIZE] ;


  private:
    uint8_t number_of_tasks;

};
#endif