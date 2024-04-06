#include "Steering.h"


Steering::Steering(uint8_t taskID, SLog *log, Blackbox *bb, HardwareSerial *visBus)
  : TaskAbstract(taskID, log, bb, visBus) {
  _tname = "STEER";
  logger->info("Steering:: initialized", _tname);
}

bool Steering::begin(Receiver *receiver) {
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("Steering:: no receiver object available", _tname);
    setError(getID(), 0x01);
    return false;
  }
  sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
  logger->info(buffer, _tname);
  return true;
}

void Steering::update(void) {

  /** if one gimbal is not centered, no optical flow measurement is needed **/
  centered[0] = _recv->isGimbalCentered(ROLL, true);
  centered[1] = _recv->isGimbalCentered(PITCH, true);
  centered[2] = _recv->isGimbalCentered(YAW, true);
  //
  /** we need difference from center position to curent position **/
  rpy[0] = GIMBAL_CENTER_POSITION - _recv->getData(ROLL);
  rpy[1] = GIMBAL_CENTER_POSITION - _recv->getData(PITCH);
  rpy[2] = GIMBAL_CENTER_POSITION - _recv->getData(YAW);

  /** if all gimbals centered , we have nothing to do and set channels relative values to 0 **/
  if (centered[0] && centered[1] && centered[2]) {
    _bbd.data.ch[ROLL] = _bbd.data.ch[PITCH] = _bbd.data.ch[YAW] = 0;
  }
  /** yaw not centered and roll&pitch is centered, than we adjust roll/pitch to get a better performance **/
  else if (!centered[2] && centered[0] && centered[1]) {
    _bbd.data.fdata[0] = rpy[2] * STEERING_ROLL_BIAS;
    _bbd.data.fdata[1] = rpy[2] * STEERING_PITCH_BIAS;
    _bbd.data.fdata[2] = rpy[2];

    _bbd.data.ch[ROLL] = uint16_t(_bbd.data.fdata[0]);
    _bbd.data.ch[PITCH] = uint16_t(_bbd.data.fdata[1]);

    /** avoid overshooting for roll/pitch **/
    _bbd.data.ch[ROLL] = constrain(_bbd.data.ch[ROLL], -STEEIRNG_MAX_RP, STEEIRNG_MAX_RP);
    _bbd.data.ch[PITCH] = constrain(_bbd.data.ch[PITCH], -STEEIRNG_MAX_RP, STEEIRNG_MAX_RP);
    _bbd.data.ch[YAW] = rpy[2];

#if defined(LOG_TASK_STEERING) || defined(LOG_TASK_ALL)
    sprintf(buffer, "RPY (%d\t%d\t%d\t) DATA(RPY) (%d\t%d\t%d\t)",
            rpy[0], rpy[1], rpy[2],
            _bbd.data.ch[ROLL], _bbd.data.ch[PITCH], _data.ch[YAW]);
    logger->info(buffer, _tname);
#endif
  } else {
#if defined(LOG_TASK_STEERING) || defined(LOG_TASK_ALL)
    sprintf(buffer, "Steering::RPY (%d\t%d\t%d\t)", rpy[0], rpy[1], rpy[2]);
    logger->info(buffer, _tname);
#endif
  }
#if defined(LOG_VISUALIZER)
    _tgroup = "UPD";
    send2Visualizer(_tname, _tgroup, &_bbd);
#endif
  // if function to the end, assumption is, that internal data struct was updated
  setUpdateFlag();
  resetError();
  yield();
}
