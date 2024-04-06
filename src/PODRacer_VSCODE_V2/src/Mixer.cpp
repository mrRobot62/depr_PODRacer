#include "Mixer.h"

// Mixer::Mixer(SLog *log, char *name, CoopSemaphore *taskSema, uint8_t mock) 
//   : PODRacer(log, name, taskSema, mock) {
    
// }

void Mixer::begin() {
  tdw = new TaskData();
}

void Mixer::update(TaskData *data) {
  data->data.start_millis = millis();
  this->tdr = data;
  // make a copy of input data. Later on, we do some updates for tdw based on task data
  memcpy(tdw, tdr, sizeof(tdr));



  delay(10);
  data->data.end_millis = millis();
  log->data(data, name, "UPD", true);
}

TaskData *Mixer::getMockedData(TaskData *td, uint8_t mode) {
  if (mode > 0) {

  }
  return this->bbd;
}