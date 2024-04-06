#ifndef _Steering_H_
#define _Steering_H_

#include <Arduino.h>
#include "Task.h"
#include <PID_v1.h>
#include "constants.h"
#include "Receiver.h"

/**
  Steering

  this class is responsible if pilot use YAW to turn the POD Racer to left or right.
  to get a better (smoother) flight, the PODracer use ROLL & PITCH & YAW for a turning
  and adjust ROLL / PITCH based on YAW value

  to stop the PODRacer, thrust should be go to 0 and PITCH a little bit backwards until
  OFLOW go to reverse

  YAW     | THRUST  | ROLL    | PITCH   
  -----------------------------------
  left    | ignore  | left    | fwd
  right   | ignore  | right   | fwd   
  center  | ignore  | center  | center
  center  | 0       | center  | backward

**/
class Steering : public TaskAbstract {
  public:
    Steering(uint8_t taskID, SLog *log, Blackbox *bb=nullptr, HardwareSerial *visBus=nullptr);

    /** initialize **/
    bool begin(void) {;};
    bool begin(Receiver *receiver);

    /** update site loop **/
    void update(void);

  private:
    Receiver *_recv;
    SimpleKalmanFilter *skfSteering;
    uint8_t centered[3];
    uint16_t rpy[3];

};

#endif