#include "Hover.h"


  Hover::Hover(uint8_t taskID, SLog *log) : TaskAbstract(taskID, log)  {
    logger->info("Hover initialized");
  }

  bool Hover::begin(Receiver *receiver) {
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("Hover:: no receiver object available");
      setError(getID());
      return false;
    }
    sprintf(buffer, "HOVER ready | Receiver:%d |", (long)&_recv);
    logger->info(buffer);
    return true;
  }

  void Hover::update(void) {

    // if function to the end, assumption is, that internal data struct was updated
    setUpdateFlag();
    resetError();
  }
