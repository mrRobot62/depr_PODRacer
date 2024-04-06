#ifndef _TASK_DATA_H_
#define _TASK_DATA_H_

#define NUMBER_CHANNELS 16
#define DATA_SIZE 8

// this struct is used to store data from tasks and transfer data from task to task and into Blackbox
// or transfer data via WIFI to host computer
typedef struct {
  uint16_t  header;               // general header two bytes
  long      millis;               // store current milliseconds
  char      fwversion[10];        // version number of firmware

  // data storage
  uint8_t   task_id;              // this task stores this data
  bool      updated;              // true, if data are updated
  bool      failsafe;             // true, if receiver recognized failsafe mode
  bool      lost_frame;           // true, if receiver recognized lost data frames
  bool      is_armed;             // true, if PODRacer is ARMED
  uint8_t   groupA;               // can be used to group data 1. order (usefull for later analysis)
  uint8_t   groupB;               // can be used to group data 2. order (usefull for later analysis)
  uint16_t  ch[NUMBER_CHANNELS];  // channel data (0=ch1, 1=ch2, 2=ch3, ...)
  long      ldata[DATA_SIZE];     // can be used to store number of long values
  long      fdata[DATA_SIZE];     // can be used to store number of float values
  long      pid_rpyth[5];         // can be used to store number of float values
  uint16_t  const_hover[4];       // 0=minimut height to start hovering, 1=min hovering heigt, 2=ideal hovering height, 3=max hovering height
  uint16_t  crc;                  // CRC-Sum
} TaskDataStruct;

typedef union {                   
  TaskDataStruct data;                  // the data struct above
  byte bytes [sizeof(TaskDataStruct)];  // Now we know how many bytes are stored
} TaskData;                                  // this union is used to store data onto SD-Card or transport via WIFI. 

#endif