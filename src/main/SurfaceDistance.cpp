#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus, Blackbox *bb)
  : TaskAbstract(taskID, log,bb) {
  _bus = bus;
  _tname = "SDIST";
  logger->info("SurfaceDistance::initialized", _tname);
}

bool SurfaceDistance::begin(Receiver *receiver) {
  _tof = new VL53L0X();
  if (_tof == nullptr) {
    logger->error("VL53L0X not initialized", _tname);
    setError(getID());
    return false;
  }
  Wire.begin();
  _tof->setTimeout(500);
  if (!_tof->init())
  {
    Serial.println("Failed to detect and initialize VL53L0X!"), _tname;
    setError(getID());
    return false;
  }
  _lidar = new TFMPlus();
  if (_lidar == nullptr) {
    logger->error("SurfaceDistance::Lidar not initialized", _tname);
    return false;
  }
  if (_bus == nullptr) {
    logger->error("SurfaceDistance::Lidar-SerialPort not initialized", _tname);
    return false;
  }

  /*
    RX1_PIN and TX1_PIN occupy the IO ports for the internal flash memory. 
    When flashing the program, GPIO2 must not be connected to the TFMini.
  */
  _bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  if (_lidar->begin(_bus) == false) {
    logger->error("SurfaceDistance::Initialization lidar sensor failed", _tname);
    return false;
  }
  delay(500);
#if defined(LOG_TASK_SURFACE1)
  sprintf(buffer, "SurfaceDistance::Lidar FW: %1u.%1u.%1u", _lidar->version[0], _lidar->version[1], _lidar->version[2]);
  logger->info(buffer, _tname);
#endif

  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("SurfaceDistance:: no receiver object available", _tname);
    return false;
  }

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).

  // <tbd> set to 50ms, normal a task run in 10ms 
  _tof->startContinuous(50);

  sprintf(buffer, "SurfaceDistance:: ready | Receiver:%d |", (long)&_recv);
  logger->info(buffer, _tname);


  resetUpdateFlag();
  resetError();
  return true;
}

  /**
    update TData for this TFMini sensor

  **/
void SurfaceDistance::update(void) {
  if (_lidar->getData(tfDist, tfFlux, tfTemp))  // Get data from the device.
  {
    channelData = _recv->getData(TASK_SURFACEDISTANCE);
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(TFMini):%04icm, Flux:%05i, CHData:%4.0f", tfDist, tfFlux, channelData);
    logger->info(buffer, _tname);
#endif
    /**** Berechnungen auf Basis TFMini ****/
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(TFMini):CHData(in):%4.0f", channelData);
    logger->info(buffer);
  } else  // If the command fails...
  {
    logger->info("SurfaceDistance:: getData failed", _tname);
    _lidar->printFrame();  // display the error and HEX data
#endif
  }
  tofMm = _tof->readRangeContinuousMillimeters();
  /**** Berechnungen auf Basis VL53L1X ****/
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(Tof):%05imm, CHData(out):%4.0f", tofMm, channelData);
    logger->info(buffer);
#endif  
  _recv->setNewData(TASK_SURFACEDISTANCE, channelData);
  setUpdateFlag();
  resetError();
}
