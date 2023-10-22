#ifndef _SURFACE_DIST_H_
#define _SURFACE_DIST_H_
#include "Arduino.h"
#include "Task.h"
#include "constants.h"
#include "Receiver.h"
#include <PID_v1.h>
#include <TFMPlus.h>
#include <Wire.h>
#include <VL53L1X.h>

class SurfaceDistance : public TaskAbstract {
  public:
    SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus);

    /** initialize **/
    bool begin(void) {;};
    bool begin(Receiver *receiver);

    /** update site loop **/
    void update(void);

  private:
    HardwareSerial *_bus;
    TFMPlus *_lidar;
    VL53L1X *_tof;
    Receiver *_recv;
    SimpleKalmanFilter *skfToF, *skfLidar;

    int16_t tfDist = 0;    // Distance to object in centimeters
    int16_t tfFlux = 0;    // Strength or quality of return signal
    int16_t tfTemp = 0;    // Internal temperature of Lidar sensor chip    
};

#endif