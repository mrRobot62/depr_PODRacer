#include "Hover.h"


  Hover::Hover(uint8_t taskID, SLog *log, Blackbox *bb) : TaskAbstract(taskID, log, bb)  {
    _tname = "HOVER";
    logger->info("initialized", _tname);
  }

  bool Hover::begin(Receiver *receiver) {
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("no receiver object available", _tname);
      setError(getID(),0x01);
      return false;
    }
    sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
    logger->info(buffer, _tname);
    clearStruct();
    sprintf(buffer, "reset hover buffer: FW: %s, TaskID:%d, Armed:%d, ch[0]:%d", 
      _bbd.data.fwversion,
      _bbd.data.task_id,
      _bbd.data.isArmed,
      _bbd.data.ch[0]
    );
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
      _bbd.data.ch[AUX2] = GIMBAL_MIN;
      _bbd.data.ch[AUX3] = GIMBAL_MIN;
      
      _blackbox->update(&_bbd);
      #if defined (RUN_HOVER)
        _bbd.data.updated = true;
      #else
        // note: if Hovering is deaktivated, all input from Receiver is send 1:1 back to all other 
        //        tasks. This is NOT A PODRacer behaviour, because RPY is not handeld by hovering
        _bbd.data.updated = false;
      #endif
      #if defined(LOG_TASK_HOVER) &  !defined(RUN_HOVER)
        if (_bbd.data.updated) {          
          sprintf(buffer, "NO HOVERING => R:%4d, P:%4d, H:%4d, Y:%4d, T:%4d, ",
            _bbd.data.ch[ROLL],
            _bbd.data.ch[PITCH],
            _bbd.data.ch[THRUST],
            _bbd.data.ch[HOVERING],
            _bbd.data.ch[YAW]
          );
          logger->info(buffer, _tname);
        }
      #endif
      #if defined(LOG_TASK_HOVER)
        if (_bbd.data.updated) {          
          sprintf(buffer, "RUNNING => R:%4d, P:%4d, H:%4d, Y:%4d, T:%4d, ",
            _bbd.data.ch[ROLL],
            _bbd.data.ch[PITCH],
            _bbd.data.ch[THRUST],
            _bbd.data.ch[HOVERING],
            _bbd.data.ch[YAW]
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
