#ifndef _HOVER_H_
#define _HOVER_H_

#include <Arduino.h>
#include "Task.h"
#include <PID_v1.h>
#include "constants.h"
#include "Receiver.h"


class Hover : public TaskAbstract {
  public:
    Hover(uint8_t taskID, SLog *log,Blackbox *bb=nullptr);

    /** initialize **/
    bool begin(void) {;};
    bool begin(Receiver *receiver);

    /** update site loop **/
    void update(void);

  private:
    Receiver *_recv;
    SimpleKalmanFilter *skfHover;
};

#endif