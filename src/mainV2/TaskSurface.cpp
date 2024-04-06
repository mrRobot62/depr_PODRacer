#include "TaskSurface.h"

TaskSurface::TaskSurface(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {

}

void TaskSurface::begin(uint8_t preventLogging ) {

}

void TaskSurface::update(TaskData *data, uint8_t preventLogging) {

}