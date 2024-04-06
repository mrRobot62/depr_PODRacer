#include "Mixer.h"

// Mixer::Mixer(SLog *log, char *name, CoopSemaphore *taskSema, uint8_t mock) 
//   : PODRacer(log, name, taskSema, mock) {
    
// }

void Mixer::begin() {

}

void Mixer::update(TaskData *data) {
  this->tdr = data;
}

TaskData *Mixer::getMockedData(TaskData *td, uint8_t mode) {
  if (mode > 0) {

  }
  return this->bbd;
}