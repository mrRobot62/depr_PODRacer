#include "Hover.h"


  Hover::Hover(uint8_t taskID, SLog *log) : TaskAbstract(taskID, log)  {
    logger->info("Hover initialized");
  }

  bool Hover::begin(Receiver *receiver) {
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("Hover:: no receiver object available");
      return false;
    }
    logger->info("Hover ready");
    return true;
  }

  void Hover::update(void) {

  }
