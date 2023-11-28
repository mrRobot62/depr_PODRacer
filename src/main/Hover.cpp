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
    #if defined(LOG_TASK_HOVER)
      logger->debug("Hover::update(void)", _tname);
    #endif
    if (_recv->isArmed()) {
      setUpdateFlag();
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
      _bbd.data.const_hover[0] = SDIST_MINIMAL_HEIGHT;
      _bbd.data.const_hover[1] = SDIST_MIN_DISTANCE;
      _bbd.data.const_hover[2] = SDIST_MAX_DISTANCE;
      _blackbox->update(&_bbd);
      _bbd.data.updated = true;
      #if defined(LOG_TASK_HOVER)
        sprintf(buffer, "RUNNING => R:%4d, P:%4d, H:%4d, Y:%4d, T:%4d",
          _bbd.data.ch[ROLL],
          _bbd.data.ch[PITCH],
          _bbd.data.ch[HOVERING],
          _bbd.data.ch[YAW],
          _bbd.data.ch[THRUST]
        );
        logger->info(buffer, _tname); 
      #endif
      #if defined(LOG_VISUALIZER) 
        //  ["TIME", "TASK","GROUP","ARMING", "CH_R", "CH_P", "CH_Y", "CH_H", "CH_T",
        //  "float0","float1", "float2", "float3", "float4", "float5", "float6", "float7",
        //  "ldata0","ldata1","ldata2","ldata3","ldata4","ldata5","ldata6","ldata7"
        //  ]
        _tgroup="UPD";
        send2Visualizer(_tname, _tgroup, &_bbd);
      #endif
    }
    else {
      #if defined(LOG_TASK_HOVER)
        logger->debug("DISARMED", _tname);
      #endif
    }
    // if function to the end, assumption is, that internal data struct was updated
    // and written to flight controller
    resetError();
  }
