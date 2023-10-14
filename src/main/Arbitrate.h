#ifndef _ARBITRATE_H_
#define _ARBITRATE_H_

#include <Arduino.h>
#include "Task.h"
#include "Receiver.h"
#include "constants.h"


  typedef struct sbus_data SDATA;  
  struct sbus_data {
    bool active;
    bool failsafe;
    bool lostframe;
    static constexpr int8_t NUM_CH = 16;
    int16_t ch[NUM_CH];        
  }; 

  class Arbitrate: public TaskAbstract {
    public:
      Arbitrate(SLog *log);
      bool begin(void);
      void update(void);

      inline bool begin(Receiver *receiver) {
        if (receiver) {
          _recv = receiver;
          logger->info("Arbitrate.begin(receiver) started, receiver: ", false);
          logger->print((long)&_recv, true);
          return true;
        }
        else {
          logger->error("Arbitrate.begin(receiver) failed");
        }
        return false;  
      }

      /** get data struct **/
      inline SDATA data() const {return _data;}
      inline void data(SDATA data) {_data = data;}

    private: 
      char buffer[100];
      SDATA _data;
      Receiver *_recv;
  };

#endif