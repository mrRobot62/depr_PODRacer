#include "SurfaceDistance.h"

SurfaceDistance::SurfaceDistance(uint8_t taskID, SLog *log, HardwareSerial *bus) : TaskAbstract(taskID, log)  {
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

  _bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  _lidar->begin(_bus);
  delay(500);
  sprintf(buffer, "Lidar FW: %1u.%1u.%1u", _lidar->version[0],_lidar->version[1],_lidar->version[2]);
  logger->info(buffer);
  
  _recv = receiver;
  if (_recv == nullptr) {
    logger->error("SurfaceDistance:: no receiver object available");
    return false;
  }
  
  logger->info("SurfaceDistance ready");
  return true;
}

void SurfaceDistance::update(void) {

}
