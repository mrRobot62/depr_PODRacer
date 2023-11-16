<<<<<<< Updated upstream
#include "HardwareSerial.h"
=======
#include "SimpleKalmanFilter.h"
>>>>>>> Stashed changes
#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus, Blackbox *bb)
  : TaskAbstract(taskID, log,bb) {
  _bus = bus;
  _tname = "SDIST";
  logger->info("initialized", _tname);
}

bool SurfaceDistance::begin(Receiver *receiver) {
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("no receiver object available", _tname);
    setError(getID(), 0x01);
    return false;
  }
  sprintf(buffer, "begin() - ready | AddrRecv:%d |", (long)&receiver);
  logger->info(buffer, _tname);

  #if defined(RUN_SDIST_VL53L0) | defined(RUN_SDIST_VL53L1)

  #if defined(RUN_SDIST_VL53L0)
    _tof = new VL53L0X();
  #elif defined(RUN_SDIST_VL53L1)
    //_tof = new VL53L1X();
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
<<<<<<< Updated upstream
=======
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
>>>>>>> Stashed changes

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
#if defined(LOG_TASK_SURFACE1)
  sprintf(buffer, "Lidar FW: %1u.%1u.%1u", _lidar->version[0], _lidar->version[1], _lidar->version[2]);
  logger->info(buffer, _tname);
#endif
*/
  _tof->startContinuous(50);

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

  /**
    update TData for this TFMini sensor

  **/
void SurfaceDistance::update(void) {
  /*
  if (_lidar->getData(tfDist, tfFlux, tfTemp))  // Get data from the device.
  {
    channelData = _recv->getData(TASK_SURFACEDISTANCE);
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(TFMini):%04icm, Flux:%05i", tfDist, tfFlux);
    logger->info(buffer, _tname);
#endif
*/
  if (_recv->isArmed()) {
    tofMm = _tof->readRangeContinuousMillimeters();
    tofMm = constrain(tofMm, SDIST_COND_MIN_VALUE, SDIST_COND_MAX_VALUE);
    // if something happens, we set channel to zero (no additional load for HOVERING)
    _bbd.data.ch[HOVERING] = 0;
    _bbd.data.ldata[0] = tofMm;     // store raw value
    if (tofMm < SDIST_MINIMAL_HEIGHT) {
        #if defined(LOG_TASK_SURFACE_TOF)
          // if minimal height is not reached no height calculation is needed
          sprintf(buffer, "TO LOW HEIGHT (%4imm,%4imm,%4imm)\t%5imm", 
          (long)SDIST_MINIMAL_HEIGHT,
          (long)SDIST_MIN_DISTANCE,
          (long)SDIST_MAX_DISTANCE,
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
    tofRawValue = (double)tofMm;
    tofPIDAdjValue = 0.0;
    // PID Controll save adjusted value into tofPIDAdjValue
    // this value is used by KalmanFilter to smoothing the real output
    pidTOF->Compute();

//    tofSKFValue = skfToF->updateEstimate(tofRawValue);
    tofSKFValue = skfToF->updateEstimate(tofPIDAdjValue);
    // now we have a PID adjusted value which was smoothed
    _bbd.data.fdata[0] = tofSKFValue;     // store raw value
    // to get a good hovering channel value, we multiply this value
    // by an bias (default is 1.0 (do not reduce/increase the value))
    hoverValue = (uint16_t)(tofSKFValue * SDIST_BIAS); 
    _bbd.data.ldata[1] = hoverValue;                // store hovering (should same as channel[HOVERING])
    _bbd.data.ch[HOVERING] = hoverValue;
    #if defined(LOG_TASK_SURFACE_TOF)
      #if defined(USE_SERIAL_PLOTTER)
        sprintf(buffer, "Raw:%i, Filtered:%4.3f, PID:%4.3f, Hover: %i", 
          tofMm,
          tofSKFValue,
          tofPIDAdjValue,
          hoverValue
        );
      #else
        sprintf(buffer, "TOF: (%4imm) HEIGHT (%4imm, %4imm), PID(SKF_IN:%+7.2f, OUT:%+7.2f, SETP:%+7.2f) => CH[HOVERING]: %4d", 
          tofMm,
          SDIST_MIN_DISTANCE, SDIST_MAX_DISTANCE,
          tofSKFValue,
          tofPIDAdjValue,
          tofSetPoint,
          hoverValue
        );
      #endif
      logger->info(buffer, _tname);
    #endif

<<<<<<< Updated upstream
/*
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(TFMini):CHData(in):%4.0f", channelData);
    logger->info(buffer);
  } else  // If the command fails...
  {
    logger->info("getData failed", _tname);
    _lidar->printFrame();  // display the error and HEX data
#endif
  }
*/
  tofMm = _tof->readRangeContinuousMillimeters();

  /**** Berechnungen auf Basis VL53L1X ****/

#if defined(LOG_TASK_SURFACE1)
    //sprintf(buffer, "Dist(Tof):%05imm, CHData(out):%4.0f", tofMm, channelData);
    sprintf(buffer, "Dist(Tof):%05imm", tofMm);
    logger->info(buffer, _tname);
#endif  
  //_recv->setNewData(TASK_SURFACEDISTANCE, channelData);
  Serial.println("-1-");
  setUpdateFlag();
=======
    //_bbd.data.fdata[0] = tofPIDAdjValue;
    setUpdateFlag();
  } else {
    resetUpdateFlag();
  }
>>>>>>> Stashed changes
  resetError();
  Serial.println("-2-");
}
