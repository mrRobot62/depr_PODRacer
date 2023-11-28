#ifndef _TASKDATA_H_
#define _TASKDATA_H_


/**
  every task use a copy of this struct and
  set specific data calculated by this task

  During arbitrating depending on the priority
  the task tdata struct is used to calculate the
  data for the mixer

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
**/


/**
  every task use a copy of this struct and
  set specific data calculated by this task

  During mixing depending on the priority
  the task tdata struct is used to calculate the
  data for the mixer

  This struct can be stored as debug data onto SDCard
**/

#define NUMBER_CHANNELS 16
#define DATA_SIZE 8

typedef struct {
  uint16_t header;
  long millis;                          // milliseconds
  char fwversion[10];                   // versionnumber

  // data
  uint8_t task_id;                      // produces by task-id
  bool updated;                         // set by tasks
  bool failsafe;                        // set by receiver
  bool lost_frame;                      // set by receiver
  bool isArmed;                         // true=armed, false=disarmed
  uint8_t groupA;                       // can be used to group data 1. order
  uint8_t groupB;                       // can be used to group data 2. order
  uint16_t ch[NUMBER_CHANNELS];         // channel data (0=ch1, 1=ch2, 2=ch3, ...)
  long ldata[DATA_SIZE];                // an be used for long values  
  double fdata[DATA_SIZE];              // can be used for float values
  double pid_rpyth[5];                  //(R/P/Y/T/H);
  uint16_t const_hover[4];              // 0=minimut height to start hovering, 1=min hovering heigt, 2=ideal hovering height, 3=max hovering height
  uint16_t crc;
} BlackBoxStruct;

typedef union {
  BlackBoxStruct data;
  byte bytes [sizeof(BlackBoxStruct)];
} BBD;

#endif