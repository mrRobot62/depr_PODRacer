#include "TaskSteering.h"

TaskSteering::TaskSteering(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {

}

void TaskSteering::begin(bool allowLog) {

}

void TaskSteering::update(TaskData *data, uint8_t preventLogging) {

}