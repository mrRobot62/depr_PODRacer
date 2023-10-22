#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log) : TaskAbstract(taskID, log)  {
  logger->info("SurfaceDistance initialized");
}

bool SurfaceDistance::begin(Receiver *receiver) {
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("SurfaceDistance:: no receiver object available");
    return false;
  }

  sprintf(buffer, "SurfaceDistance ready | Receiver:%d |", (long)&_recv);
  logger->info(buffer);
  return true;
}

void SurfaceDistance::update(void) {


}
