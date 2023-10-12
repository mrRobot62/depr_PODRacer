#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define LOGLEVEL 3  // 3=Info, 4=Debug

//#define LOG_TASK_ALL
//#define LOG_TASK_RECEIVER
#define LOG_TASK_ARBITRATE
//#define LOG_TASK_OPTICALFLOW
//#define LOG_TASK_SURFACE1
//#define LOG_TASK_SURFACE2


#define LED_BUILTIN 15
#define LOOP_TIME 20
#define HB_BLINK_FREQ 250
#define PIN_PMW3901 5

// arbitration flag as a 16bit flag
#define HB 1
#define MOVEMENT 2
#define DISTANCE1 4
#define DISTANCE2 8
#define RECEIVER 0x10

// receivers toogle values around 1-5 +/- if nothing done by user activity
// this avoid false readings inside Receiver-class
#define RECEIVER_NOISE_MIN -5       // lower range e.g mid postion is 1500 - but receiver is noicy and move between 1498-1500 (lower) => min=-2
#define RECEIVER_NOISE_MAX 5       // higher range e.g mid postion is 1500 - but receiver is noicy and move between 1500-1503 (lower) => max=+3

#endif