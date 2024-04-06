/*




*/
#ifndef _MIXER_H_
#define _MIXER_H_
#include "PODRacer.h"

class Mixer : public PODRacer {
  public:
    Mixer(SLog *log, char *name, CoopSemaphore *taskSema = nullptr)
      : PODRacer(log, name, taskSema) {;};
    // ~Mixer() {};
    void init() {;};
    void begin(void);
    void update(TaskData *data); 

  protected:
    TaskData *getMockedData(TaskData *td, uint8_t mode);

  protected:
    TaskData *tdr = nullptr;
    TaskData *tdw = nullptr;

};
#endif