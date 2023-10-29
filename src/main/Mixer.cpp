#include "Mixer.h"


Mixer::Mixer(uint8_t taskID, SLog *logger, Blackbox *bb) : TaskAbstract(taskID, logger, bb)  {
  _tname = "MIXER";
}

bool Mixer::begin(void) {
  bool rc = false;
  resetError();
  return true;
}

void Mixer::update(OpticalFlow *obj) {

}
void Mixer::update(Hover *obj) {

}
void Mixer::update(SurfaceDistance *obj) {

}
void Mixer::update(Steering *obj) {

}

void Mixer::update() {
  if (_recv) {
  _recv->write();
  }
  else {
      logger->error("Mixer.update() - no receiver object", _tname);
  }

  #ifdef LOG_TASK_MIXER || LOG_TASK_ALL
    logger->debug("Mixer::update()", _tname);
  #endif
}

