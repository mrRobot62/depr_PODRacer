#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define FW_VERSION_PATTERN "%02d.%02d.%02d"
#define FW_VERSION_MAJOR 0
#define FW_VERSION_MINOR 1
#define FW_VERSION_PATCH 0

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


#define LOGLEVEL 3  // 3=Info, 4=Debug

//#define LOG_TASK_ALL
#define LOG_TASK_RECEIVER_W
//#define LOG_TASK_RECEIVER_R
//#define LOG_TASK_RECEIVER
#define LOG_TASK_MIXER
//#define LOG_TASK_HOVER
//#define LOG_TASK_OPTICALFLOW
#define LOG_TASK_SURFACE1
//#define LOG_TASK_SURFACE2
//#define LOG_TASK_STEERING
//#define LOG_FILE_LOGGER
//#define USE_SERIAL_PLOTTER


//#define TEST_OPTICAL_FLOW



#define LED1 15
#define LED_BUILTIN 2
#define LOOP_TIME 10
#define HB_BLINK_FREQ 250
#define PIN_PMW3901 5

// TASK_IDs are used to indicate a blink pattern
#define TASK_HB 0
#define TASK_HOVER 1
#define TASK_OPTICALFLOW 2
#define TASK_SURFACEDISTANCE 3
#define TASK_FRONTDISTANCE 4
#define TASK_STEERING 5
#define TASK_RECEIVER 6
#define TASK_MIXER 7
#define TASK_EMERGENCY 8

// receivers toogle values around 1-5 +/- if nothing done by user activity
// this avoid false readings inside Receiver-class. As bigger the value is, as less sensitive your PODracer is around center gimbal position
#define RECEIVER_NOISE 5       // if normaly center pos should be 1500 (calibrated transmitter), mostley this value slip +/- 5 aournd this centerposition
                                // is used inside receiver to remove this slip

#define GIMBAL_CENTER_POSITION (int16_t)1500
#define GIMBAL_MIN (int16_t)1000
#define GIMBAL_MAX (int16_t)2000

#define ROLL  0  // internal channel mapping set A(Roll) on position 0
#define PITCH 1  // dito for E(Pitch)
#define THRUST 7  // dito for T(Throttle) -> forward THRUST ESC -> we used the throttle gimbal for flying forward
#define YAW 3      // dito for R(Yaw)
#define ARMING 4   // ch5 for arming/disarming
#define AUX1 4
#define AUX2 5
#define AUX3 6
#define HOVERING 2     // hovering -> all 4 motors Throttle - flight controller manage this

// range interval around centerpositoin for A/E/Y
// this is used by sensors like OpticalFlow. All Values between value-CENTER_RANGE and value+CENTER_RANGE are used as "CENTER"
// this avoid very sensible gimbal movements. For a PODRacer this is ok, for real quad copters a totally NO_GO ;-)
// value is used for isGimbalMin/Max
#define CENTER_RANGE 50 

// Hovering
#define HOVER_ROLL GIMBAL_CENTER_POSITION   // default ch1
#define HOVER_PITCH GIMBAL_CENTER_POSITION  // default ch2
#define HOVER_YAW GIMBAL_CENTER_POSITION    // default ch4
#define HOVER_THRUST GIMBAL_MIN             // ch8 forward mapped used for ESC for EDF thrust nozzle

//
// OpticalFlow
#define FLOW_COUNTER_MAX 100
#define SLIP_RANGE 500
#define PMW3901_ZERO 1
#define PID_OUTPUT_LIMIT 250

// Steering
#define STEERING_ROLL_BIAS 0.2
#define STEERING_PITCH_BIAS 0.2
#define STEEIRNG_MAX_RP 150
// TFMini Serial 1
#define RX1_PIN 2
#define TX1_PIN 4

// Blackbox
#define BLACKBOX_CS_PIN 32
#endif