#include "Arbitrate.h"

namespace arbi {

Arbitrate::Arbitrate(SLog *logger) : TaskAbstract(logger)  {

}

bool Arbitrate::begin(void) {
  bool rc = false;
  logger->warn("Arbitrate.begin(void) not implemented yet");
  return rc;
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

}