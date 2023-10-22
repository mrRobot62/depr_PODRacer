#ifndef _MIXER_H_
#define _MIXER_H_

#include <Arduino.h>
#include "Task.h"
#include "Receiver.h"
#include "OpticalFlow.h"
#include "SurfaceDistance.h"
#include "Hover.h"
#include "constants.h"


  typedef struct sbus_data SDATA;  
  struct sbus_data {
    bool active;
    bool failsafe;
    bool lostframe;
    static constexpr int8_t NUM_CH = 16;
    int16_t ch[NUM_CH];        
  }; 

  class Mixer: public TaskAbstract {
    public:
      Mixer(uint8_t taskID, SLog *log);
      bool begin(void);
      void update(void);

      void update(OpticalFlow *obj);
      void update(SurfaceDistance *obj);
      void update(Hover *obj);
  
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

      /** add value to current channel value **/
      void addChannelValue(uint8_t ch, uint16_t value) {
        _data.ch[ch] = +value;
      }

      /** set value current channel value (overwrite) **/
      void setChannelValue(uint8_t ch, uint16_t value) {
         _data.ch[ch] = value;
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