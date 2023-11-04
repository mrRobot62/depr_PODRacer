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
    Hovering is the absolutely base task. In this case Hovering is always updated and the complete channels are
    copied into the mixer struct (absolute values)

    All other tasks works only with relativ values. These values are added or subtracted to the absolute values

  **/
  if (_recv && _hover->data().data.isArmed) {
    if (_hover->isUpdated() || _sdist->isUpdated()) {  
      _HoverMixer();
    }
    /**
      FlowTask has a lower priority as SteeringTask, because flow control is only used, if the PODRacer is hovering without moving 
      IF FlowTask is updated, SteeringTask is ignored
    **/
    if (_flow->isUpdated()) {
      _RPYMixer();
    } else if (_steer->isUpdated()){
      _RPYMixer();
    }
    // 
    // move(ref) mixer data struct to receiver and write back to flight controller 
    memcpy(_recv->data().data.ch, _bbd.data.ch, sizeof(_bbd.data.ch));
    _recv->write();
  }
  else {
      logger->error("Mixer.update() - no receiver object", _tname);
  }

}

/** used by HoverTask & SurfaceDistanceTask **/
void Mixer::_HoverMixer(void) {
    memcpy(_bbd.data.ch, _hover->data().data.ch, sizeof(_bbd.data.ch));
    #if defined(TASK_MIXER)    
      sprintf(buffer, "R:%d, P:%d, H:%d, Y:%d, Arm:%d, TH:%d  - Hover:%d - SDist: %d (_HoverMixer)",
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
    // if surface distance is not perfect over ground, than we adjust
    // the current HOVERING value.
    // adding a value means PODRacer to low over ground
    // substract a value means PODRacer to high over ground
    _bbd.data.ch[HOVERING] += _sdist->data().data.ch[HOVERING];
}

/** used by FlowTask & SteeringTask **/
void Mixer::_RPYMixer(void) {
  _bbd.data.ch[ROLL] += _sdist->data().data.ch[ROLL];
  _bbd.data.ch[PITCH] += _sdist->data().data.ch[PITCH];
  _bbd.data.ch[YAW] += _sdist->data().data.ch[YAW];

  #if defined(TASK_MIXER)    
    sprintf(buffer, "R:%d, P:%d, H:%d, Y:%d, Arm:%d, TH:%d - Hover:%d - SDist: %d (_RPYMixer)",
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

