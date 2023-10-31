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
  if (_recv) {
    if (_hover->isUpdated() && _hover->data().data.isArmed) {  
      #if defined(TASK_MIXER)    
        sprintf(buffer, "R:%d, P:%d, H:%d, Y:%d, Arm:%d, TH:%d (HOVER)",
          _hover->data().data.ch[0],
          _hover->data().data.ch[1],
          _hover->data().data.ch[2],
          _hover->data().data.ch[3],
          _hover->data().data.ch[4],
          _hover->data().data.ch[7]
        );
        logger->info(buffer,_tname);
      #endif
      memcpy(_bbd.data.ch, _hover->data().data.ch, sizeof(_bbd.data.ch));
    }
    // <tbd> OpticalFlow

    // <tbd> Steering

    // <tbd> SurfaceDistance

    
    if (_bbd.data.isArmed) {
      // move(ref) mixer data struct to receiver and write back to flight controller 
      memcpy(_recv->data().data.ch, _bbd.data.ch, sizeof(_bbd.data.ch));
      _recv->write();
    }
  }
  else {
      logger->error("Mixer.update() - no receiver object", _tname);
  }

}

