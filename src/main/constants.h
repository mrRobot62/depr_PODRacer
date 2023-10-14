#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define LOGLEVEL 3  // 3=Info, 4=Debug

//#define LOG_TASK_ALL
//#define LOG_TASK_RECEIVER
//#define LOG_TASK_ARBITRATE
#define LOG_TASK_OPTICALFLOW
//#define LOG_TASK_SURFACE1
//#define LOG_TASK_SURFACE2
#define USE_SERIAL_PLOTTER

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
// this avoid false readings inside Receiver-class. As bigger the value is, as less sensitive your PODracer is around center gimbal position
#define RECEIVER_NOISE 5       // if normaly center pos should be 1500 (calibrated transmitter), mostley this value slip +/- 5 aournd this centerposition
                                // is used inside receiver to remove this slip

#define GIMBAL_CENTER_POSITION (int16_t)1500
#define GIMBAL_MIN (int16_t)1000
#define GIMBAL_MAX (int16_t)2000

#define AILERON  0  // internal channel mapping set A(Roll) on position 0
#define ELEVATOR 1  // dito for E(Pitch)
#define THROTTLE 2  // dito for T(Throttle)
#define YAW 3       // dito for R(Yaw)
#define AUX1 4
#define AUX2 5
#define AUX3 6
#define AUX4 6

// range interval around centerpositoin for A/E/Y
// this is used by sensors like OpticalFlow. All Values between value-CENTER_RANGE and value+CENTER_RANGE are used as "CENTER"
// this avoid very sensible gimbal movements. For a PODRacer this is ok, for real quad copters a totally NO_GO ;-)
#define CENTER_RANGE 50 

//
// OpticalFlow
#define MOVE_RANGE 500



#endif