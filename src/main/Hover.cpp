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
    last_value = 0;
    return true;
  }

  void Hover::update(void) {

    if (_recv->isArmed()) {
      _bbd.data.task_id = _id;
      _bbd.data.millis = millis();
      _bbd.data.updated = true;             // for saftey, set updated to true to avoid a blackout
      _bbd.data.ch[ROLL] = HOVER_ROLL;
      _bbd.data.ch[PITCH] = HOVER_PITCH;
      _bbd.data.ch[HOVERING] = _recv->getData(HOVERING);
      _bbd.data.ch[THRUST] = HOVER_THRUST;
      _bbd.data.ch[YAW] = HOVER_YAW;
      _blackbox->update(&_bbd);
      if (_bbd.data.ch[HOVERING] != last_value) {
        _bbd.data.updated = true;
        last_value = _bbd.data.ch[HOVERING]; 
      }
      else {
        _bbd.data.updated = false;
      }

      #if defined(LOG_TASK_HOVER)
        if (_bbd.data.updated) {          
          sprintf(buffer, "R:%4d P:%4d T:%4d H:%4d Y:%4d",
            _bbd.data.ch[ROLL],
            _bbd.data.ch[PITCH],
            _bbd.data.ch[THRUST],
            _bbd.data.ch[HOVERING],
            _bbd.data.ch[YAW]
          );
          logger->debug(buffer, _tname);
          sprintf(buffer, "H:%4d",
            _bbd.data.ch[HOVERING]
          );
          logger->info(buffer, _tname);  
        }
      #endif
    }
    else {
      #if defined(TASK_HOVER)
        logger->debug("DISARMED", _tname);
      #endif
      last_value = 0;
    }
    // if function to the end, assumption is, that internal data struct was updated
    // and written to flight controller
    resetError();
  }
