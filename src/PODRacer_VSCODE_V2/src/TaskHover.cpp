#include "TaskHover.h"

TaskHover::TaskHover(SLog *log, const char*name, uint8_t taskID, CoopSemaphore *taskSema) 
  : Task(log, name, taskID, taskSema) {
    //bbd = new TaskData();
}

void TaskHover::begin(bool allowLog) {
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

void TaskHover::update(bool armed, bool allowLog) {
  if (armed) {
    resetTaskData();
    log->once_warn(&log_once_mask, LOG_ONCE_DATA1_BIT,"PODRacer armed", name);
    bbd->data.start_millis = millis();
    bbd->data.ch[ROLL] = HOVER_ROLL;
    bbd->data.ch[PITCH] = HOVER_PITCH;
    bbd->data.ch[HOVERING] = getGlobalChannel(HOVERING);
    bbd->data.ch[THRUST] = HOVER_THRUST;
    bbd->data.ch[YAW] = HOVER_YAW;
    bbd->data.ch[AUX2] = GIMBAL_MIN;
    bbd->data.ch[AUX3] = GIMBAL_MIN;
    bbd->data.const_hover[0] = SDIST_MINIMAL_HEIGHT;
    bbd->data.const_hover[1] = SDIST_MIN_DISTANCE;
    bbd->data.const_hover[2] = SDIST_MAX_DISTANCE;
    bbd->data.updated = true;
    bb->update(bbd);
    bbd->data.end_millis = millis();
    Serial.print("HOVER ");
    log->data(bbd, allowLog, name, "UPD");
  }
  else {
    log->once_warn(&log_once_mask, LOG_ONCE_WARN0_BIT,"PODRacer disarmed", name);
  }
}