#include "TaskOpticalFlow.h"

TaskOpticalFlow::TaskOpticalFlow(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {

}

void TaskOpticalFlow::begin(uint8_t preventLogging) {

}

void TaskOpticalFlow::update(TaskData *data, uint8_t preventLogging) {

}