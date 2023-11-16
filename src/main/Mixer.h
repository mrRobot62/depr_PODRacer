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
      bool begin(void) {
        bool rc = false;
        resetError();
        return true;        
      };
      void update(void);
      void update(OpticalFlow *obj);
      void update(SurfaceDistance *obj);
      void update(Hover *obj);
      void update(Steering *obj);

      inline bool begin(Receiver *receiver) {
        if (receiver) {
          _recv = receiver;
        }
        else {
          logger->error("begin() - failed - receiver missing");
          return false;
        }

        sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
        logger->info(buffer, _tname);
        return true;  
      }

    private:
      void _HoverMixer(uint8_t taskId);
      void _RPYMixer(void);

    private: 
      char buffer[100];
      char _hs = 0x00;

      //SDATA _data;
      Receiver *_recv;
      OpticalFlow *_flow;
      Hover *_hover;
      SurfaceDistance *_sdist;
      Steering *_steer;

  };

#endif