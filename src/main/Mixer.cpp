#include "Mixer.h"


Mixer::Mixer(uint8_t taskID, SLog *logger) : TaskAbstract(taskID, logger)  {

}

bool Mixer::begin(void) {
  bool rc = false;
  logger->warn("Mixer.begin(void) not implemented yet");
  resetError();
  return rc;
}

void Mixer::update(OpticalFlow *obj) {

}
void Mixer::update(Hover *obj) {

}
void Mixer::update(SurfaceDistance *obj) {

}

void Mixer::update() {
  if (_recv) {
  _recv->write();
  }
  else {
      logger->error("Mixer.update() - no receiver object");
  }

  #ifdef LOG_TASK_MIXER || LOG_TASK_ALL
    logger->debug("Mixer::update()");
  #endif
}

