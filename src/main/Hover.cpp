#include "Hover.h"


  Hover::Hover(uint8_t taskID, SLog *log, Blackbox *bb) : TaskAbstract(taskID, log, bb)  {
    _tname = "HOVER";
    logger->info("initialized", _tname);
  }

  bool Hover::begin(Receiver *receiver) {
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("no receiver object available", _tname);
      setError(getID());
      return false;
    }
    sprintf(buffer, "ready | Receiver:%d |", (long)&_recv);
    logger->info(buffer, _tname);
    return true;
  }

  void Hover::update(void) {

    _bbd.data.task_id = _id;
    _bbd.data.millis = millis();
    _bbd.data.updated = true;
    _bbd.data.ch[ROLL] = HOVER_ROLL;
    _bbd.data.ch[PITCH] = HOVER_PITCH;
    _bbd.data.ch[THROTTLE] = HOVER_THROTTLE;
    _bbd.data.ch[THRUST] = HOVER_THRUST;
    _bbd.data.ch[YAW] = HOVER_YAW;
    _blackbox->update(&_bbd);

    // if function to the end, assumption is, that internal data struct was updated
    // and written to flight controller
    resetError();
  }
