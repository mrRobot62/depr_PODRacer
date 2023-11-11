#include "HardwareSerial.h"
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
    sprintf(buffer, "Dist(TFMini):%04icm, Flux:%05i, CHData:%4.0f", tfDist, tfFlux, channelData);
    logger->info(buffer, _tname);
#endif
*/

    /**** Berechnungen auf Basis TFMini ****/

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
  resetError();
  Serial.println("-2-");
}
