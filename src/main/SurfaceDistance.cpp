#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus)
  : TaskAbstract(taskID, log) {
  _bus = bus;
  logger->info("SurfaceDistance initialized");
}

bool SurfaceDistance::begin(Receiver *receiver) {
  _lidar = new TFMPlus();
  if (_lidar == nullptr) {
    logger->error("Lidar not initialized");
    return false;
  }
  if (_bus == nullptr) {
    logger->error("Lidar-SerialPort not initialized");
    return false;
  }

  /*
    RX1_PIN and TX1_PIN occupy the IO ports for the internal flash memory. 
    When flashing the program, GPIO2 must not be connected to the TFMini.
  */
  _bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  if (_lidar->begin(_bus) == false) {
    logger->error("Initialization lidar sensor failed");
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
    return false;
  }

  logger->info("SurfaceDistance ready");
  return true;
}

  /**
    update TData for this TFMini sensor

  **/
void SurfaceDistance::update(void) {
  if (_lidar->getData(tfDist, tfFlux, tfTemp))  // Get data from the device.
  {
#if defined(LOG_TASK_SURFACE1)
    sprintf(buffer, "Dist:%04icm, Flux:%05i", tfDist, tfFlux);
    logger->info(buffer);
#endif
    _data.ch[THROTTLE] = tfDist;
#if defined(LOG_TASK_SURFACE1)
  } else  // If the command fails...
  {
    logger->info("SurfaceDistance getData failed");
    _lidar->printFrame();  // display the error and HEX data
#endif
  }
}
