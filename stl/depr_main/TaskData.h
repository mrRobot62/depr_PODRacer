#ifndef _TASKDATA_H_
#define _TASKDATA_H_


/**
  every task use a copy of this struct and
  set specific data calculated by this task

  During arbitrating depending on the priority
  the task tdata struct is used to calculate the
  data for the mixer

**/
typedef struct tdata TDATA;  
struct tdata {
  uint8_t id;
  bool updated;
  uint8_t cmd;
  uint16_t rawX, rawY;
  bool failsafe;
  bool lost_frame;
  static constexpr int8_t NUM_CH = 16;
  int16_t ch[NUM_CH];
        
};

#endif