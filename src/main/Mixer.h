#ifndef _MIXER_H_
#define _MIXER_H_

#include <Arduino.h>
#include "Task.h"
#include "Receiver.h"
#include "OpticalFlow.h"
#include "SurfaceDistance.h"
#include "Hover.h"
#include "Steering.h"
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
      Mixer(uint8_t taskID, SLog *log, Blackbox *bb=nullptr);
      bool begin(void);
      void update(void);

      void update(OpticalFlow *obj) {_flow = obj; update();};
      void update(SurfaceDistance *obj) {_sdist = obj;update();}
      void update(Hover *obj) {_hover = obj;update();}
      void update(Steering *obj) {_steer = obj;update();}

      inline bool begin(Receiver *receiver) {
        if (receiver) {
          _recv = receiver;
          logger->info("begin(receiver) started, receiver: ", _tname);
          logger->print((long)&_recv, true);
          return true;
        }
        else {
          logger->error("Arbitrate.begin(receiver) failed", _tname);
        }
        return false;  
      }


    private: 
      char buffer[100];
      //SDATA _data;
      Receiver *_recv;
      OpticalFlow *_flow;
      Hover *_hover;
      SurfaceDistance *_sdist;
      Steering *_steer;

  };

#endif