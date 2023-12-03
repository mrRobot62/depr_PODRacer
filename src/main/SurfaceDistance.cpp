#include "HardwareSerial.h"
#include "SimpleKalmanFilter.h"
#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus, Blackbox *bb)
  : TaskAbstract(taskID, log,bb) {
  _bus = bus;
  _tname = "SDIST";
  logger->info("initialized", _tname);
}

// --------------------------------------------------------------------------------------------
// BEGIN - initalize this task
// --------------------------------------------------------------------------------------------
bool SurfaceDistance::begin(Receiver *receiver) {
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("no receiver object available", _tname);
    setError(getID(), 0x01);
    return false;
  }
  sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
  logger->info(buffer, _tname);

  #if defined(USE_SDIST_VL53L0) | defined(USE_SDIST_VL53L1)

  #if defined(USE_SDIST_VL53L0)
    _tof = new VL53L0X();
  #elif defined(USE_SDIST_VL53L1)
    _tof = new VL53L1X();
  #endif

  if (_tof == nullptr) {
    logger->error("VL53L0/1X not initialized", _tname);
    setError(getID(), 0x02);
    return false;
  }
  Wire.begin();
  // please note: if no device is attached a call of _tof->init() will crash system (reboot)
  if (!_tof->init())
  {
    logger->error("Failed to detect and initialize VL53L0/1X!", _tname);
    setError(getID(),0x03);
    return false;
  }
  _tof->setTimeout(500);
  _tof->startContinuous(SDIST_CONT_SCANS_MS);
  pidTOF = new PID(&tofSKFValue, &tofPIDAdjValue, &tofSetPoint, kpTOF, kiTOF, kdTOF, DIRECT);

  if (pidTOF == nullptr) {
    logger->error("PID controller not initialized", _tname);
    setError(getID(), 0x03);
    return false;
  }

  pidTOF->SetMode(AUTOMATIC);
  pidTOF->SetOutputLimits(-SDIST_PID_OUTPUT_LIMIT, SDIST_PID_OUTPUT_LIMIT);
  pidTOF->SetSampleTime(LOOP_TIME);

  skfToF = new SimpleKalmanFilter(skfeMea, skfeEst, skfE);

  
  // ---------------------------------------------------------
  // setup TMMini LIDAR sensor
  // ---------------------------------------------------------

  /*
  _lidar = new TFMPlus();
  if (_lidar == nullptr) {
    logger->error("Lidar not initialized", _tname);
    setError(getID(), 0x04);
    return false;
  }
  if (_bus == nullptr) {
    logger->error("Lidar-SerialPort not initialized", _tname);
    setError(getID(), 0x05);
    return false;
  }
*/


  /*
    RX1_PIN and TX1_PIN occupy the IO ports for the internal flash memory. 
    When flashing the program, GPIO2 must not be connected to the TFMini.
  */

/*
  _bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  if (_lidar->begin(_bus) == false) {
    logger->error("Initialization lidar sensor failed", _tname);
    setError(getID(), 0x06);
    return false;
  }
  delay(500);
#if defined(LOG_TASK_SURFACE_LIDAR)
  sprintf(buffer, "Lidar FW: %1u.%1u.%1u", _lidar->version[0], _lidar->version[1], _lidar->version[2]);
  logger->info(buffer, _tname);
#endif

*/

  // ---------------------------------------------------------
  // setup VL53L0/1 sensor
  // ---------------------------------------------------------

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
    
  // <tbd> set to 50ms, normal a task run in 10ms 

  #endif // RUN_SDIST_VL53L0 | RUN_SDIST_VL53L1
  resetUpdateFlag();
  resetError();
  return true;
}

// --------------------------------------------------------------------------------------------
// UPDATE - this method is called continousley by scheduler (see main.cpp - task functions)
// --------------------------------------------------------------------------------------------
void SurfaceDistance::update(void) {
/**
  SurfaceDistance use two different sensors (TFMini & VL53Lxy). The TFMini is in front of the
  PODRacer, the other one in the back.

  Theoretically both sensors should measure the same distance, but obviosluy that's not a typical
  szenario.

  Inside this methode, we calculate a average between both values and put them together into the 
  PIDController to 

  Important Notes to understand Visualization-Output (and storing data into task data structure)

  General
  ch[0..8]  stores only a releative deviation from the actual mean value (1500)
  TOF
  fdata [0] PID output
  fdata [1] HOVER Value - this represent the relativ value in the channel (as doublt)
  fdata [2] PID Setpoint
  fdata [3] Kalman filter output value

  ldata [0] TOF raw value 
  ldata [1] HOVER value (same as ch[HOVER])
  ldata [2] 0 - not used
  ldata [3] 0 - not used

  LIDAR
  fdata [4] PID output
  fdata [5] HOVER Value - this represent the relativ value in the channel
  fdata [6] PID Setpoint
  fdata [7] Kalman filter output value
  
**/


  if (_recv->isArmed()) {
/*
    if (_lidar->getData(tfDist, tfFlux, tfTemp))  // Get data from the device.
    {
  #if defined(LOG_TASK_SURFACE_LIDAR)
      sprintf(buffer, "Dist(TFMini):%04icm, Flux:%05i", tfDist, tfFlux);
      logger->info(buffer, _tname);
  #endif
    }

*/
    setUpdateFlag();
    setMilliseconds();
    tofMm = _tof->readRangeContinuousMillimeters();
    tofMm = constrain(tofMm, SDIST_COND_MIN_VALUE, SDIST_COND_MAX_VALUE);
    // if something happens, we set channel to zero (no additional load for HOVERING)
    _bbd.data.ch[HOVERING] = 0;
    _bbd.data.ldata[SDIST_LDATA_TOF_RAW] = (long)tofMm;     // store raw value
    //sprintf(buffer, "(1) RAW: %imm \tldata: %i \tSP:{%6.2f}", tofMm, _bbd.data.ldata[SDIST_LDATA_TOF_RAW], tofSetPoint);
    //logger->info(buffer, _tname);
    if (tofMm < SDIST_MINIMAL_HEIGHT) {
        #if defined(LOG_TASK_SURFACE_TOF)
          // if minimal height is not reached no height calculation is needed
          sprintf(buffer, "<<<<<<<<< HEIGHT TO LOW (%4imm,%4imm,%imm, %4imm)\tRAW: %5imm <<<<<<<<<", 
            (long)SDIST_MINIMAL_HEIGHT,
            (long)SDIST_MIN_DISTANCE,
            (long)tofSetPoint,
            (long)SDIST_MAX_DISTANCE,
            //_bbd.data.ldata[SDIST_LDATA_TOF_RAW]
            tofMm
          );
          logger->info(buffer, _tname);
        #endif      
        return;
    }
    
    //
    // If PODRacer is higher as MINIMAL_HEIGHT, than we use a PID-Controller to
    // put it into the correct height
    tofMm = constrain(tofMm, SDIST_MINIMAL_HEIGHT, SDIST_COND_MAX_VALUE);
    //sprintf(buffer, "(2) RAW: %imm \tldata: %i \tSP:{%6.2f}", tofMm, _bbd.data.ldata[SDIST_LDATA_TOF_RAW], tofSetPoint);
    //logger->info(buffer, _tname);
    tofRawValue = (double)tofMm;  // convert to double
    // this raw value is filtered to remove jitter
    // this value is used by KalmanFilter to smoothing the real output
    tofSKFValue = skfToF->updateEstimate(tofRawValue);
    tofPIDAdjValue = 0.0;
    // PID Controll save adjusted value into tofPIDAdjValue
    // Input is the filtered value from kalman and output is saved into tofPIDAdjValue

    // Parameters for PID(&tofSKFValue, &tofPIDAdjValue, &tofSetPoint, kpTOF, kiTOF, kdTOF, DIRECT);
    pidTOF->Compute();
    sprintf(buffer, "SKF: %6.2.f\tPID-IN:%6.2f\tPID-OUT:%6.2f", tofSKFValue, tofSKFValue, tofPIDAdjValue);
    logger->info(buffer, _tname);
    // now we have a PID adjusted value which was smoothed
  
    _bbd.data.fdata[SDIST_FDATA_TOF_PID] = tofPIDAdjValue;                      // store adjusted (OUT) value from PID
    _bbd.data.fdata[SDIST_FDATA_TOF_HOVER] = (tofPIDAdjValue * SDIST_BIAS);     // adjust a little bit this value with a bias 
    _bbd.data.fdata[SDIST_FDATA_TOF_SETPOINT] = tofSetPoint;                    // set point for TOF-PID
    _bbd.data.fdata[SDIST_FDATA_TOF_SKFVALUE] = tofSKFValue;                    // store calculated SKF value from raw input data
    
    _bbd.data.ldata[SDIST_LDATA_TOF_HOVER] = (int16_t)_bbd.data.fdata[SDIST_FDATA_TOF_HOVER];                // store hovering (should same as channel[HOVERING])
    _bbd.data.ch[HOVERING] = _bbd.data.ldata[SDIST_LDATA_TOF_HOVER];
    setUpdateFlag();
    #if defined(LOG_TASK_SURFACE_TOF)
      #if defined(USE_SERIAL_PLOTTER)
        sprintf(buffer, "TOF:%i, SKF:%.2f, PID:%f, SP:%.2f, Hover:%i", 
          _bbd.data.ldata[SDIST_LDATA_TOF_RAW],
          tofSKFValue,
          _bbd.data.fdata[SDIST_FDATA_TOF_PID],
          tofSetPoint,
          _bbd.data.ldata[SDIST_LDATA_TOF_HOVER]
        );
        logger->simulate(buffer);
      #else
        sprintf(buffer, "TOF: (%4imm) HEIGHT (%4imm, %4imm), PID(SKF_IN:%+7.2f, OUT:%+7.2f, SETP:%+7.2f) => CH[HOVERING]: %4d", 
          _bbd.data.ldata[SDIST_LDATA_TOF_RAW],
          SDIST_MIN_DISTANCE, SDIST_MAX_DISTANCE,
          tofSKFValue,
          _bbd.data.fdata[SDIST_FDATA_TOF_PID],
          tofSetPoint,
          _bbd.data.ldata[SDIST_LDATA_TOF_HOVER]
        );
      logger->info(buffer, _tname);
      #endif
    #endif

    setMilliseconds();

    #if defined(LOG_VISUALIZER) 
      _tgroup="UPD";
      send2Visualizer(_tname, _tgroup, &_bbd);
    #endif

    #if defined(IGNORE_SDIST_OUTPUT_TOF) 
      resetUpdateFlag();
    #endif
    //_bbd.data.fdata[0] = tofPIDAdjValue;
  } else {
    resetUpdateFlag();
  }
  resetError();
}
