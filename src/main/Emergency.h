#include <Arduino.h>
#include "Task.h"
#include "TaskData.h"
#include "Receiver.h"


class Emergency : public TaskAbstract {
  public:
    Emergency(uint8_t taskID, SLog *log, Blackbox *bb=nullptr): TaskAbstract(taskID, log,bb) {
  _tname = "EMERGENCY";
  logger->info("initialized", _tname);
}

    /** initialize **/
    bool begin(void) {;};
    bool begin(Receiver *receiver) {
      _recvOk = false;
      if (receiver != nullptr) {
        _recvOk = true;
      }
    }

    /** update site loop **/
    void update(void);

    bool isArmed(void) {
      return _recv->isArmed();
    }

    bool isDisarmed(void) {
      return !isArmed();
    }
    
    bool isLostFrame(void) {
      return _recv->isLostFrame();
    } 

    bool isLostFrame(void) {
      return _recv->isFailSave();
    }


  private:
    Receiver *_recv;
    bool _recvOk;

}