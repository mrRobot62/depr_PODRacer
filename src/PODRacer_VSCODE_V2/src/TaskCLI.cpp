#include "TaskCLI.h"

TaskCLI::TaskCLI(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {
    //bbd = new TaskData();
    cli_enabled = false;
}

void TaskCLI::begin(bool allowLog) {
  sprintf(buffer, "begin() - task ready - allowLogging: %d", allowLog);
  log->info(buffer, true, name);

  resetTaskData();
  sprintf(buffer, "reset hover buffer: FW: %s, TaskID:%d, Armed:%d, ch[HOVERING]:%d, ",
          bbd->data.fwversion,
          bbd->data.task_id,
          bbd->data.is_armed,
          bbd->data.ch[HOVERING]);
  log->info(buffer, true, _tname);  
}

void TaskCLI::update(bool armed, bool allowLog) {
  if (armed) {
    cli_enabled = false;
    resetTaskData();
    log->once_warn(&log_once_mask, LOG_ONCE_DATA1_BIT,"PODRacer armed - CLI deactivated", name);
  }
  else {
    cli_enabled = true;
    log->once_warn(&log_once_mask, LOG_ONCE_WARN0_BIT,"PODRacer disarmed - CLI activated", name);
  }
}