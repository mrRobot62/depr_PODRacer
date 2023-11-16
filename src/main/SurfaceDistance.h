#ifndef _SURFACE_DIST_H_
#define _SURFACE_DIST_H_
#include "Arduino.h"
#include "Task.h"
#include "constants.h"
#include "Receiver.h"
#include <PID_v1.h>
#include <TFMPlus.h>
#include <Wire.h>
#if defined(RUN_SDIST_VL53L0)
  #include <VL53L0X.h>
#elif defined(RUN_SDIST_VL53L1)
  #include <VL53L1X.h>
#endif

class SurfaceDistance : public TaskAbstract {
  public:
    SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus, Blackbox *bb=nullptr);

    /** initialize **/
    bool begin(void) {;};
    bool begin(Receiver *receiver);

    /** update site loop **/
    void update(void);

  private:
    HardwareSerial *_bus;
    TFMPlus *_lidar;
#if defined(RUN_SDIST_VL53L0)
    VL53L0X *_tof;
#elif defined(RUN_SDIST_VL53L1)
    VL53L1X *_tof;
#endif


    Receiver *_recv;
    SimpleKalmanFilter *skfToF, *skfLidar;

    double skfE = 0.005;      // we assume a loop in 50ms, to be adjusted if filter is not good enough
    double skfeMea = 1.0;      // intial for SKF, is adjusted during runtime
    double skfeEst = skfeMea;  // is adjusted during runtime
    int16_t tfDist = 0;    // Distance to object in centimeters
    int16_t tfFlux = 0;    // Strength or quality of return signal
    int16_t tfTemp = 0;    // Internal temperature of Lidar sensor chip
    int16_t tofMm = 0;     // Distance tof to object in millimeters
    int16_t hoverValue=0;


    //----- PID Controller for SDIST sensor
    PID *pidTOF, *pidLIDAR;
    double tofSetPoint = (SDIST_MAX_DISTANCE-SDIST_MIN_DISTANCE)/2 + SDIST_MIN_DISTANCE; 
    double tofRawValue, tofSKFValue, tofPIDAdjValue;

    double kpTOF  = 0.8;
    double kiTOF  = 0.005;     //0.9
    double kdTOF  = 0.01;     //3.6

};

#endif