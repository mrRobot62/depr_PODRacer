#include "Arbitrate.h"


Arbitrate::Arbitrate(uint8_t taskID, SLog *logger) : TaskAbstract(taskID, logger)  {

}

bool Arbitrate::begin(void) {
  bool rc = false;
  logger->warn("Arbitrate.begin(void) not implemented yet");
  return rc;
}

void Arbitrate::update(OpticalFlow *obj) {

}
void Arbitrate::update(Hover *obj) {

}
void Arbitrate::update(SurfaceDistance *obj) {

}
void Arbitrate::update() {
  if (_recv) {
  _recv->write();
  }
  else {
      logger->error("arbitrate.update() - no receiver object");
  }

  #ifdef LOG_TASK_ARBITRATE || LOG_TASK_ALL
    logger->debug("Arbitrate::update()");
  #endif
}

