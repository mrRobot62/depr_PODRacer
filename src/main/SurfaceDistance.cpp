#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus)
  : TaskAbstract(taskID, log) {
  _bus = bus;
  logger->info("SurfaceDistance initialized");
}

bool SurfaceDistance::begin(Receiver *receiver) {
  _tof = new VL53L1X();
  if (_tof == nullptr) {
    logger->error("VL53L1X not initialized");
    setError(getID());
    return false;
  }
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C
  _tof->setTimeout(500);
  if (!_tof->init())
  {
    Serial.println("Failed to detect and initialize VL53L1X!");
    setError(getID());
    return false;
  }
  _lidar = new TFMPlus();
  if (_lidar == nullptr) {
    logger->error("Lidar not initialized");
    setError(getID());
    return false;
  }
  if (_bus == nullptr) {
    logger->error("Lidar-SerialPort not initialized");
    setError(getID());
    return false;
  }

  /*
    RX1_PIN and TX1_PIN occupy the IO ports for the internal flash memory. 
    When flashing the program, GPIO2 must not be connected to the TFMini.
  */
  _bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  if (_lidar->begin(_bus) == false) {
    logger->error("Initialization lidar sensor failed");
    setError(getID());
    return false;
  }
  delay(500);
#if defined(LOG_TASK_SURFACE1)
  sprintf(buffer, "Lidar FW: %1u.%1u.%1u", _lidar->version[0], _lidar->version[1], _lidar->version[2]);
  logger->info(buffer);
#endif

  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("SurfaceDistance:: no receiver object available");
    setError(getID());
    return false;
  }

  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  _tof->setDistanceMode(VL53L1X::Long);
  _tof->setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  _tof->startContinuous(50);

  logger->info("SurfaceDistance ready");
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
    logger->info(buffer);
#endif
    /**** Berechnungen auf Basis TFMini ****/
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist(TFMini):CHData(in):%4.0f", channelData);
    logger->info(buffer);
  } else  // If the command fails...
  {
    logger->info("SurfaceDistance getData failed");
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
