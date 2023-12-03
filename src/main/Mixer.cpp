#include "Mixer.h"


Mixer::Mixer(uint8_t taskID, SLog *logger, Blackbox *bb) : TaskAbstract(taskID, logger, bb)  {
  _tname = "MIXER";
}

bool Mixer::begin(Receiver *receiver) {
  if (receiver) {
    _recv = receiver;
  }
  else {
    logger->error("begin() - failed - receiver missing");
    return false;
  }

  sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
  logger->info(buffer, _tname);


  return true;  
}

void Mixer::update(Hover *obj) {
  /** 
    Hovering is the absolutely base task. As a base task after hovering the data struct is allways filled with absolute values.
    All other tasks works only with relativ values. These values are added or subtracted to the absolute values

  **/
  _hover = obj;
  if (_recv->isArmed()) {
      _HoverMixer(TASK_HOVER);
  }
  update();
}

void Mixer::update(SurfaceDistance *obj) {
  /**
    SurfaceDistance is responsible for the exakt height of the PODRacer. This task check if a minimal height was reached
    and than, it will manage the throttle management
  **/
  _sdist = obj;
  if (_recv->isArmed()) {
    #if defined(USE_SDIST_OUTPUT)
      _HoverMixer(TASK_SURFACEDISTANCE);
    #endif
  }
  update();
}

void Mixer::update(OpticalFlow *obj) {
  _flow = obj; 
  if (_recv->isArmed()) {
    Serial.println("update(OpticalFlow *obj)");
  }
  update();
};

void Mixer::update(Steering *obj) {
  _steer = obj;
  if (_recv->isArmed()) {
    Serial.println("update(Steering *obj)");
  }
  update();
}

void Mixer::update() {
  _recv->write(&_bbd);

/*
  if (_recv) {
    if (_recv->isArmed()) {
      _hs = 0;
      logger->debug("RUN_HOVER", _tname);
      logger->info("-1-", _tname);
      _HoverMixer(TASK_HOVER);
      logger->info("-2-", _tname);
      sprintf(buffer, "_sdist addr: %d", (long)&_sdist);
      logger->info(buffer);
      sprintf(buffer, "_sdist->isUpdated: %d", _sdist->data().data.updated);
      logger->info(buffer);
      if (_sdist->isUpdated()) {
        logger->info("-2a-", _tname);
        _HoverMixer(TASK_SURFACEDISTANCE);
        logger->info("-2b-", _tname);
      }
      logger->info("-3-", _tname);

      //
      //  FlowTask has a lower priority as SteeringTask, because flow control is only used, if the PODRacer is hovering without moving 
      //  IF FlowTask is updated, SteeringTask is ignored
      //
      #if defined(RUN_OPTICALFLOW)
        logger->info("RUN_OPTICALFLOW", _tname);
        if (_flow->isUpdated()) {
          _RPYMixer();
        } 
        else {
      #endif
      #if defined(RUN_STEERING)
        logger->info("RUN_STEERING", _tname);
        if (_steer->isUpdated()){
          _RPYMixer();
        #endif
      #if defined(RUN_OPTICALFLOW)
      }
      #endif
      // 
      // move(ref) mixer data struct to receiver and write back to flight controller 
      #if defined(LOG_TASK_MIXER)
        logger->info("memcpy to recv", _tname);
      #endif
      #if defined(LOG_TASK_MIXER) | defined(LOG_TASK_HOVER)
        sprintf(buffer, "MIXER DATA R:%4d, P:%4d, H:%4d, Y:%4d, Arm:%4d, TH:%4d",
          _bbd.data.ch[0],
          _bbd.data.ch[1],
          _bbd.data.ch[2],
          _bbd.data.ch[3],
          _bbd.data.ch[4],
          _bbd.data.ch[7]
        );
        logger->info(buffer,_tname);
      #endif       
      //logger->info("before _recv->write");
      _recv->write(&_bbd);
      //logger->info("after _recv->write");
    }
    else {
      #if defined(LOG_TASK_MIXER)
        logger->warn("DISARMED", _tname);
      #endif
    }
  }
  else {
      logger->error("Mixer.update() - no receiver object", _tname);
      setError(getID(), 0x01);
  }
  */
}

/** used by HoverTask & SurfaceDistanceTask **/
void Mixer::_HoverMixer(uint8_t taskId = TASK_HOVER) {  
  if (taskId == TASK_HOVER) {
    memcpy(_bbd.data.ch, _hover->data().data.ch, sizeof(_bbd.data.ch));
  }
  else if (taskId == TASK_SURFACEDISTANCE) {
    // nothing special for SDIST preparation
  }
  else {
    sprintf(buffer,"invalid taskId used (%d) ", taskId);
    logger->error(buffer);
  }
    #if defined(LOG_TASK_MIXER_HOVER) | defined(LOG_TASK_MIXER_SDIST)
      sprintf(buffer, "TASK(%2d) R:%4d, P:%4d, H:%4d, Y:%4d, Arm:%4d, TH:%4d",
        taskId,
        _bbd.data.ch[0],
        _bbd.data.ch[1],
        _bbd.data.ch[2],
        _bbd.data.ch[3],
        _bbd.data.ch[4],
        _bbd.data.ch[7]
      );
      logger->info(buffer,_tname);
    #endif
    if (taskId == TASK_HOVER) {
      // do nothing, because _bbd.data.ch contain hovering base data
      return; // do nothing, because _bbd.data.ch contain hovering base data
    }

    // if surface distance is not perfect over ground, than we adjust
    // the current HOVERING value.
    // adding a value means PODRacer to low over ground
    // substract a value means PODRacer to high over ground
    if (taskId == TASK_SURFACEDISTANCE) {
      tmp1 =  _bbd.data.ch[HOVERING];
      _bbd.data.ch[HOVERING] += _sdist->data().data.ldata[SDIST_LDATA_TOF_HOVER];
    }
    _bbd.data.ch[HOVERING] = constrain(_bbd.data.ch[HOVERING], GIMBAL_MIN, GIMBAL_MAX);
      #if defined(LOG_TASK_MIXER_SDIST)
        sprintf(buffer, "SDIS ch[HOVERING](%4d) add (%4d) set (%4d)", 
          tmp1,
          _sdist->data().data.ldata[SDIST_LDATA__TOFHOVER],
          _bbd.data.ch[HOVERING]
        );
        logger->info(buffer, _tname);
      #endif
}

/** used by FlowTask & SteeringTask **/
void Mixer::_RPYMixer(void) {
  // _bbd.data.ch[ROLL] += _sdist->data().data.ch[ROLL];
  // _bbd.data.ch[PITCH] += _sdist->data().data.ch[PITCH];
  // _bbd.data.ch[YAW] += _sdist->data().data.ch[YAW];

  #if defined(LOG_TASK_MIXER_RPY)    
    sprintf(buffer, "FLOW/STEER R:%4d, P:%4d, H:%4d, Y:%4d, Arm:%4d, TH:%4d",
      _bbd.data.ch[0],
      _bbd.data.ch[1],
      _bbd.data.ch[2],
      _bbd.data.ch[3],
      _bbd.data.ch[4],
      _bbd.data.ch[7]
    );
    logger->info(buffer,_tname);
  #endif

}
