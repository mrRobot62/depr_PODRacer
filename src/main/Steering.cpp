#include "Steering.h"


  Steering::Steering(uint8_t taskID, SLog *log) : TaskAbstract(taskID, log)  {
    logger->info("Steering initialized");
  }

  bool Steering::begin(Receiver *receiver) {
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("Steering:: no receiver object available");
      setError(getID());
      return false;
    }
    sprintf(buffer, "STEERING ready | Receiver:%d |", (long)&_recv);
    logger->info(buffer);
    return true;
  }

  void Steering::update(void) {

    /** if one gimbal is not centered, no optical flow measurement is needed **/
    centered[0] = _recv->isGimbalCentered(ROLL,true);
    centered[1] = _recv->isGimbalCentered(PITCH,true);
    centered[2] = _recv->isGimbalCentered(YAW,true);
    //
    /** we need difference from center position to curent position **/
    rpy[0] = GIMBAL_CENTER_POSITION - _recv->getData(ROLL);
    rpy[1] = GIMBAL_CENTER_POSITION - _recv->getData(PITCH);
    rpy[2] = GIMBAL_CENTER_POSITION - _recv->getData(YAW);

    /** yaw not centered and roll&pitch is centered, than we adjust roll/pitch to get a bette performance **/
    if ( !centered[2] && centered[0] && centered[1] ) {
      _data.ch[ROLL] = rpy[2] * STEERING_ROLL_BIAS;
      _data.ch[PITCH] = rpy[2] * STEERING_PITCH_BIAS;

      /** avoid overshooting for roll/pitch **/
      _data.ch[ROLL] = constrain(_data.ch[ROLL], -STEEIRNG_MAX_RP, STEEIRNG_MAX_RP);
      _data.ch[PITCH] = constrain(_data.ch[PITCH], -STEEIRNG_MAX_RP, STEEIRNG_MAX_RP);
      _data.ch[YAW] = rpy[2];

      #if defined(LOG_TASK_STEERING) || defined(LOG_TASK_ALL)
        sprintf(buffer, "RPY (%d\t%d\t%d\t) DATA(RPY) (%d\t%d\t%d\t)", 
        rpy[0], rpy[1], rpy[2], 
        _data.ch[ROLL], _data.ch[PITCH], _data.ch[YAW]);
        logger->info(buffer);
      #endif
    } else {
      #if defined(LOG_TASK_STEERING) || defined(LOG_TASK_ALL)
        sprintf(buffer, "RPY (%d\t%d\t%d\t)", rpy[0], rpy[1], rpy[2]);
        logger->info(buffer);
      #endif
    }
    // if function to the end, assumption is, that internal data struct was updated
    setUpdateFlag();
    resetError();
  }
