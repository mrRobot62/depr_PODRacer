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
      : PODRacer(log, name, taskSema) {;};
    // ~Mixer() {};
    void init() {;};
    void begin(void);

    void addTask(Task  *task, uint8_t taskID){
      uint8_t id = constrain(number_of_tasks++, 0, TASK_LIST_SIZE);
      taskList.list[id] = task;
      taskList.taskID = taskID;
    } ;
  
    void update(TaskData *data); 
    
    TaskData *getTaskData() {return this->tdw;};
  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode);

  protected:
    TaskData *tdr = nullptr;
    TaskData *tdw = nullptr;

    typedef struct {
      uint8_t taskID;
      Task *list[TASK_LIST_SIZE];
    } TaskList;

    TaskList taskList ;


  private:
    uint8_t number_of_tasks;

};
#endif