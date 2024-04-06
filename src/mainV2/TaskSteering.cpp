#include "TaskSteering.h"

TaskSteering::TaskSteering(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {

}

void TaskSteering::begin(uint8_t preventLogging) {

}

void TaskSteering::update(TaskData *data, uint8_t preventLogging) {

}