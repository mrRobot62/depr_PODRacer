#include "Emergency.h"

Emergency::Emergency(uint8_t taskID, SLog *log, Blackbox *bb)
  : TaskAbstract(taskID, log, bb) {
      _tname = "EMGCY";
      logger->info("initialized", _tname);
}

bool Emergency::begin(Receiver *receiver) {
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("no receiver object available", _tname);
    return false;
  }
  sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
  logger->info(buffer, _tname);

  _recvOk=true;
  return true;
}