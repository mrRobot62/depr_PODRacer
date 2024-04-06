#include "TaskHover.h"

TaskHover::TaskHover(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {

}

void TaskHover::begin(uint8_t preventLogging) {

}

void TaskHover::update(TaskData *data, uint8_t preventLogging) {

}