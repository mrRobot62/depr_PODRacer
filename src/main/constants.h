#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define TEST_NO_TRANSMITTER_USED            // TEST - ATTENTION use this only to work withou transmiter



#define FW_VERSION_PATTERN "%02d.%02d.%02d"
#define FW_VERSION_MAJOR 0
#define FW_VERSION_MINOR 1
#define FW_VERSION_PATCH 0

#define BYTE_TO_BINARY_PATTERN  "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY_PATTERN2 "%c%c%c%c% c%c%c%c"
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

// if you use the PODRacer Visualizer / Analyzer - please use this define and comment ALL LOG_xxxxx output
#define LOG_VISUALIZER


//#define LOG_TASK_ALL
//#define LOG_TASK_RECEIVER_W
//#define LOG_TASK_RECEIVER_RRAW
//#define LOG_TASK_RECEIVER_R
//#define LOG_TASK_RECEIVER
//#define LOG_TASK_MIXER_HOVER
//#define LOG_TASK_MIXER_SDIST
//#define LOG_TASK_MIXER_RPY
//#define LOG_TASK_MIXER_RECVW
//#define LOG_TASK_MIXER_OFLOW
//#define LOG_TASK_HOVER
//#define LOG_TASK_OPTICALFLOW
//#define LOG_TASK_SURFACE_LIDAR
//#define LOG_TASK_SURFACE_TOF
//#define LOG_TASK_SURFACE_BOTH        // both values 
//#define LOG_TASK_STEERING
//#define LOG_FILE_LOGGER
//#define USE_SERIAL_PLOTTER

//#define USE_SDIST_VL53L0        // only VL53L0 OR VL53L1 - not both
#define USE_SDIST_VL53L1      // only VL53L1 OR VL53L0 - not both

//-------------------------------------------------------------
// only used during implementing to test & check outputs
// all parts should used in real behaviour !
// hovering as base task can't be ignored
//
// Note: if below parts are commented, the behind task will work
//        and can output messages, and inside MIXER the results
//        from this task will be ignored
//-------------------------------------------------------------
#define USE_SDIST_OUTPUT         
//#define USE_OFLOW_OUTPUT
//#define USE_STEERING_OUTPUT
//-------------------------------------------------------------




//#define TEST_OPTICAL_FLOW



#define LED1 15
#define LED_BUILTIN 2
#define LOOP_TIME 10
#define PIN_PMW3901 5

//------------------------------------------------------------------------------------------------------------
// TASK_IDs are used to indicate a blink pattern
//------------------------------------------------------------------------------------------------------------
#define TASK_HB 0xFF
#define TASK_HOVER 1
#define TASK_OPTICALFLOW 2
#define TASK_SURFACEDISTANCE 3
#define TASK_FRONTDISTANCE 4
#define TASK_STEERING 5
#define TASK_RECEIVER 6
#define TASK_MIXER 7
#define TASK_EMERGENCY 8

//------------------------------------------------------------------------------------------------------------
// receivers toogle values around 1-5 +/- if nothing done by user activity
// this avoid false readings inside Receiver-class. As bigger the value is, as less sensitive your PODracer is around center gimbal position
//------------------------------------------------------------------------------------------------------------
#define RECEIVER_NOISE 5       // if normaly center pos should be 1500 (calibrated transmitter), mostley this value slip +/- 5 aournd this centerposition
                                // is used inside receiver to remove this slip

#define GIMBAL_CENTER_POSITION (int16_t)1500
#define GIMBAL_MIN (int16_t)1000
#define GIMBAL_MAX (int16_t)2000

#define ARMING_VALUE 1600     // everything above this value is armed

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



//------------------------------------------------------------------------------------------------------------
// Hovering
//------------------------------------------------------------------------------------------------------------
#define HOVER_ROLL GIMBAL_CENTER_POSITION   // default ch1
#define HOVER_PITCH GIMBAL_CENTER_POSITION  // default ch2
#define HOVER_YAW GIMBAL_CENTER_POSITION    // default ch4
#define HOVER_THRUST GIMBAL_MIN             // ch8 forward mapped used for ESC for EDF thrust nozzle

//------------------------------------------------------------------------------------------------------------
//
// OpticalFlow
//------------------------------------------------------------------------------------------------------------
//#define OFLOW_COUNTER_MAX 100         // currently not used :-/
#define OFLOW_SLIP_RANGE 500          // during measurement we add the result from sensor, if sum is larger than range, we assume PODRacer slips
#define OFLOW_PMW3901_ZERO 1          // due to sensor fluctuation a little bit, this value is used as "minus/plus range" to avoid jitter
#define OFLOW_PID_OUTPUT_LIMIT 250    // to avoid to big output from PIDController, we limit the output value

//------------------------------------------------------------------------------------------------------------
// SurfaceDistance
//------------------------------------------------------------------------------------------------------------
#define SDIST_MIN_DISTANCE 400         // mm minimum height for hovering
#define SDIST_MAX_DISTANCE 500         // mm maximum height for hovering
#define SDIST_THRESHOLD_RANGE_MM 50    // mm if current range is in a time range SDIST_THRESHOLD_RANGE_MS more than the last value - USE it
#define SDIST_THRESHOLD_RANGE_MS 50    // ms observe values in this timerange
#define SDIST_CONT_SCANS_MS 50         // ms for continous scans
#define SDIST_MINIMAL_HEIGHT 100       // mm up from this height the SDIST-Task will check target height, this is a security topic
#define SDIST_COND_MIN_VALUE 0         // mm define min valid conditions for height. If environment condition is bad, sensor can deliver invalid values
#define SDIST_COND_MAX_VALUE 1000      // mm max valid condition value for height
#define SDIST_PID_OUTPUT_LIMIT 100     // to avoid to big output from PIDController, we limit the output value. Adjust this value if to fast/slow in steps of +/- 10 
#define SDIST_BIAS 0.75                // please adjust this value -> higher more throttle outcome, lower -> less throttle outcome (Multiplicator for PIDAdjustesValue. This value is the new "HOVERING-Value")

#define SDIST_LDATA_TOF_RAW 0              // index in ldata array
#define SDIST_LDATA_TOF_HOVER 1

#define SDIST_FDATA_TOF_PID 0
#define SDIST_FDATA_TOF_HOVER 1
#define SDIST_FDATA_TOF_SETPOINT 2
#define SDIST_FDATA_TOF_SKFVALUE 3

// TFMini Serial 1
#define SDIST_LDATA_LIDAR_RAW 4              // index in ldata array
#define SDIST_LDATA_LIDAR_HOVER 5

#define SDIST_FDATA_LIDAR_PID 4
#define SDIST_FDATA_LIDAR_HOVER 5
#define SDIST_FDATA_LIDAR_SETPOINT 6
#define SDIST_FDATA_LIDAR_SKFVALUE 7

#define RX1_PIN 0                       // GPIO 0 to TX on Lidar
#define TX1_PIN 2                       // GPIO 2 to RX on Lidar



//------------------------------------------------------------------------------------------------------------
// Steering
//------------------------------------------------------------------------------------------------------------
#define STEERING_ROLL_BIAS 0.1      // adjust this if during steering, this bias is not high enough (or is to high) - roll & pitch must not be the same
#define STEERING_PITCH_BIAS 0.1     // adjust this if during steering, this bias is not high enough (or is to high) - roll & pitch must not be the same
#define STEEIRNG_MAX_RP 150         // adjust this, if the maximum compensation is not optimal, as higher as more pitch/roll follow up the yaw steering

//------------------------------------------------------------------------------------------------------------
// Blackbox
//------------------------------------------------------------------------------------------------------------
#define BLACKBOX_CS_PIN 32



#endif