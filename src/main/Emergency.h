#include <Arduino.h>
#include "Task.h"
#include "Receiver.h"


class Emergency : public TaskAbstract {
  public:
    Emergency(uint8_t taskID, SLog *log, Blackbox *bb=nullptr): TaskAbstract(taskID, log, bb) {
      _tname = "EMGCY";
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

    /** return true if lost_frame or fail_safe **/
    bool isEmergencyStop(void) {
      bool rc = (isLostFrame() || isFailSafe());
      return rc;
    }

    /** update site loop **/
    void update(void) {/** nothing todo **/};

    /** return true if PODRacer is armed **/
    bool isArmed(void) {
      return _recv->isArmed();
    }

    /** return true if PODRacer is diarmed **/
    bool isDisarmed(void) {
      return !isArmed();
    }

    /** if arming is prevented return true **/
    bool isPreventArming(void) {
      return _recv->isPreventArming();
    }

    /** return true if PODRacer has lost frames **/
    bool isLostFrame(void) {
      return _recv->isLostFrame();
    } 

    /** return true if PODRacer is in fail safe mode **/
    bool isFailSafe(void) {
      return _recv->isFailSave();
    }


  private:
    Receiver *_recv;
    bool _recvOk;

};