#include "Mixer.h"


Mixer::Mixer(uint8_t taskID, SLog *logger, Blackbox *bb) : TaskAbstract(taskID, logger, bb)  {
  _tname = "MIXER";
}

bool Mixer::begin(void) {
  bool rc = false;
  resetError();
  return true;
}

void Mixer::update() {
  /** 
    Hovering is the absolutely base task and can't be deactivated in the case that the object is not available.
    The hovering object must be allways available. If RUN_HOVERING is not set, than the update flag inside Hover-Class is
    set to false. As a base task after hovering the data struct is allways filled with absolute values.

    All other tasks works only with relativ values. These values are added or subtracted to the absolute values

  **/

  //Serial.println((long)&_recv);

  if (_recv) {
    if (_recv->isArmed()) {
      _hs = 0;
      logger->info("RUN_HOVER", _tname);
      _HoverMixer(TASK_HOVER);
      #if defined(RUN_SDIST)
        logger->info("RUN_SDIST", _tname);
        if (_sdist->isUpdated()) {
          _HoverMixer(TASK_SURFACEDISTANCE);
        }
      #endif

      /**
        FlowTask has a lower priority as SteeringTask, because flow control is only used, if the PODRacer is hovering without moving 
        IF FlowTask is updated, SteeringTask is ignored
      **/
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
      logger->info("memcpy to recv", _tname);
      memcpy(_recv->data().data.ch, _bbd.data.ch, sizeof(_bbd.data.ch));
      logger->debug("write sbus", _tname);
      _recv->write();
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

}

/** used by HoverTask & SurfaceDistanceTask **/
void Mixer::_HoverMixer(uint8_t taskId = TASK_HOVER) {
  Serial.println("_HoverMixer - 1 - ");
    memcpy(_bbd.data.ch, _hover->data().data.ch, sizeof(_bbd.data.ch));
  Serial.println("_HoverMixer - 2 - ");
    #if defined(TASK_MIXER)    
//      sprintf(buffer, "R:%d, P:%d, H:%d, Y:%d, Arm:%d, TH:%d  - Hover:%d - SDist: %d (_HoverMixer)",
      sprintf(buffer, "R:%4d, P:%4d, H:%4d, Y:%4d, Arm:%4d, TH:%4d",
        _bbd.data.ch[0],
        _bbd.data.ch[1],
        _bbd.data.ch[2],
        _bbd.data.ch[3],
        _bbd.data.ch[4],
        _bbd.data.ch[7] //,
        //_hover->data().data.ch[HOVERING],
        //_sdist->data().data.ch[HOVERING]
      );
      logger->info(buffer,_tname);
    #endif
    // if surface distance is not perfect over ground, than we adjust
    // the current HOVERING value.
    // adding a value means PODRacer to low over ground
    // substract a value means PODRacer to high over ground
    #if defined(TASK_SURFACEDISTANCE)
      if (taskId == TASK_SURFACEDISTANCE) {
        _bbd.data.ch[HOVERING] += _sdist->data().data.ch[HOVERING];
      }
    #endif
}

/** used by FlowTask & SteeringTask **/
void Mixer::_RPYMixer(void) {
  _bbd.data.ch[ROLL] += _sdist->data().data.ch[ROLL];
  _bbd.data.ch[PITCH] += _sdist->data().data.ch[PITCH];
  _bbd.data.ch[YAW] += _sdist->data().data.ch[YAW];

  #if defined(LOG_TASK_MIXER)    
    sprintf(buffer, "R:%4d, P:%4d, H:%4d, Y:%4d, Arm:%4d, TH:%4d - Hover:%4d - SDist: %4d (_RPYMixer)",
      _bbd.data.ch[0],
      _bbd.data.ch[1],
      _bbd.data.ch[2],
      _bbd.data.ch[3],
      _bbd.data.ch[4],
      _bbd.data.ch[7],
      _hover->data().data.ch[HOVERING],
      _sdist->data().data.ch[HOVERING]
    );
    logger->info(buffer,_tname);
  #endif

}
