#ifndef _HOVER_H_
#define _HOVER_H_

#include <Arduino.h>
#include "Task.h"
#include <PID_v1.h>
#include "constants.h"

class Hover : public TaskAbstract {
  public:
    Hover(SLog *log);

    /** initialize **/
    bool begin(void);

    /** update site loop **/
    void update(void);

  private:
    SimpleKalmanFilter *skfHover;
};

#endif