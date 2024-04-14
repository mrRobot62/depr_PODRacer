/*



  InternalErrors:
  0x01 = 
*/
#ifndef _TSURFACE_H_
#define _TSURFACE_H_
#include "Task.h"
#include <PID_v1.h>
#include <TFMPlus.h>
#include <Wire.h>
#if defined(USE_SDIST_VL53L0)
  #include <VL53L0X.h>
#elif defined(USE_SDIST_VL53L1)
  #include <VL53L1X.h>
#endif


// LOG-ONCE mechanism only for this class
// bit 8-15 are individual for classes and are define in the class header file 
#define LOG_ONCE_SDIST_TOF1 8
#define LOG_ONCE_SDIST_TOF2 9
#define LOG_ONCE_SDIST_TOF_WARN3 10
#define LOG_ONCE_SDIST_LIDAR1 11
#define LOG_ONCE_SDIST_LIDAR2 12
#define LOG_ONCE_SDIST_LIDAR_WARN3 13
#define LOG_ONCE_SDIST_MOCK1 14
#define LOG_ONCE_SDIST_MOCK2 15

// explicit Errors in this task      
#define ERROR_TASK_PID1 1   // Problem with PID-Controller
#define ERROR_TASK_PID2 2   // Problem with PID-Controller
#define ERROR_TASK_PID3 3   // Problem with PID-Controller
#define ERROR_TASK_SKF1 10   // Problem with KalmanFilter
#define ERROR_TASK_SKF2 11   // Problem with KalmanFilter
#define ERROR_TASK_SKF3 12   // Problem with KalmanFilter
#define ERROR_TASK_TOF1 20   // Problem with KalmanFilter
#define ERROR_TASK_TOF2 21   // Problem with KalmanFilter
#define ERROR_TASK_TOF3 22   // Problem with KalmanFilter
#define ERROR_TASK_LIDAR1 30   // Problem with KalmanFilter
#define ERROR_TASK_LIDAR2 31   // Problem with KalmanFilter
#define ERROR_TASK_LIDAR3 32   // Problem with KalmanFilter

// TOF errors 0xA0..AF
#define ERROR_LIDAR_OBJ 0x20    // LIDAR errors 0xB0..BF

#define USE_TOF_SENSOR 0        // used as array index => ignore_sensor[]
#define USE_LIDAR_SENSOR 1      // used as array index => ignore_sensor[]

class TaskSurface : public Task {
  public:
    TaskSurface(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema, HardwareSerial *bus);

    void init(void) {;};      // implementation form abstract class
    void begin(bool allowLog = 0) ;      
    void update(bool allowLog = 0) {;};
    void update(bool armed, bool allowLog = 0);
    
  protected:
        
    // normally used to test the system with simulated sensor data
    // 
    TaskData *getMockedData(TaskData *td, uint8_t mode) {
      // default
      // td->data.ch[ROLL] = 0;
      // td->data.ch[PITCH] = 0;
      // td->data.ch[YAW] = 0;
      // td->data.ch[HOVERING] = 0;
      // td->data.ch[THRUST] = 0;
      // td->data.ch[AUX2] = 0;
      // td->data.ch[AUX3] = 0;
      switch(mode) {
        case 0: {
          td->data.ldata[SDIST_LDATA_LIDAR_RAW] = (long)sdistSetPoint;
          break;
        }
        case 1: {
          td->data.ldata[SDIST_LDATA_TOF_RAW] = (long)sdistSetPoint;
          break;
        }
        case 2: {
          break;
        }
      }
      return td;
    }

  private:  
    HardwareSerial *bus;
    TFMPlus *lidar;
#if defined(USE_SDIST_VL53L0)
    VL53L0X *tof;
#elif defined(USE_SDIST_VL53L1)
    VL53L1X *tof;
#endif    

    SimpleKalmanFilter *skfToF, *skfLidar;

    double skfE = 0.02;      // we assume a loop in 50ms, to be adjusted if filter is not good enough
    double skfeMea = 1.0;      // intial for SKF, is adjusted during runtime
    double skfeEst = skfeMea;  // is adjusted during runtime
    int16_t lidarDist = 0;    // Distance to object in centimeters
    int16_t lidarFlux = 0;    // Strength or quality of return signal
    int16_t lidarTemp = 0;    // Internal temperature of Lidar sensor chip
    int16_t tofMm = 0;     // Distance tof to object in millimeters
    int16_t hoverValue=0;
    int16_t rawSUM = 0;

    //----- PID Controller for SDIST sensor
    PID *pidTOF, *pidLIDAR;
    double sdistSetPoint = (SDIST_MAX_DISTANCE-SDIST_MIN_DISTANCE)/2.0 + SDIST_MIN_DISTANCE; 
    double lidarRawValue, lidarSKFValue, lidarPIDAdjValue, tofRawValue, tofSKFValue, tofPIDAdjValue;

    double kpSDIST  = 0.8;
    double kiSDIST  = 0.0;     // set a very small value because ki is slow and aggregate all errors over time
    double kdSDIST  = 0.0;       // 0.1 is not the badest value ;-)



};



#endif