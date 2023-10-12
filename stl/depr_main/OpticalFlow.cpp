#include "OpticalFlow.h"

//namespace podr {

  OpticalFlow::OpticalFlow(SLog *log, uint8_t pin) : TaskAbstract(log) {
    _pin = pin;
  }


  bool OpticalFlow::begin(void) {
    bool rc;
    flow = new Bitcraze_PMW3901(_pin);

    return rc;
  }

  /**
    update TData for this OpticalFlow sensor

  **/
  void OpticalFlow::update(void) {
    flow->readMotionCount(&deltaX, &deltaY);
  }


//};