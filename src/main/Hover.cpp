#include "Hover.h"


  Hover::Hover(SLog *log) : TaskAbstract(log)  {
    logger->info("Hover initialized");

  }

  bool Hover::begin(void) {

    logger->info("Hover ready");
    return true;
  }

  void Hover::update(void) {

  }
