#include "Receiver.h"

namespace recv {

  Receiver::Receiver(SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert) : TaskAbstract(log)  {
    _bus = bus;
    _invert = invert;
    _txpin = txpin;
    _rxpin = rxpin;

    logger->info("Receiver initialized");
  }

  bool Receiver::begin(void) {
    bool rc = false;
    sbus_rx = new bfs::SbusRx(_bus, _rxpin, _txpin, _invert);
    sbus_tx = new bfs::SbusTx(_bus, _rxpin, _txpin, _invert);

    if (sbus_rx == NULL || sbus_tx == NULL) {
      logger->error("can't create a SBus rx/tx object");
      rc = false;
      return rc;
    }
    sbus_rx->Begin();
    sbus_tx->Begin();
    rc = true;
    logger->info("Receiver.begin(void) started ");
    return rc;
  }


  void Receiver::update(void) {
    if (sbus_rx->Read()) {
      sbus_data = sbus_rx->data();
      uint16_t v1,v2;
      for (uint8_t i=0; i < _data.NUM_CH;i++) {
        _data.ch[i] = map(sbus_data.ch[i], 
                  channel_calibration[i][0], 
                  channel_calibration[i][1], 
                  channel_calibration[i][2], 
                  channel_calibration[i][3]);
      }
      _data.failsafe = sbus_data.failsafe;
      _data.lost_frame = sbus_data.lost_frame;
      _data.updated = true;

      #if defined(LOG_TASK_RECEIVER) || defined(LOG_TASK_ALL)
        sprintf(buffer, "Receiver::update() CH:<%4d, %4d, %4d, %4d> AUX:<%4d, %4d, %4d, %4d>", 
          _data.ch[0],
          _data.ch[1],
          _data.ch[2],
          _data.ch[3],
          _data.ch[4],
          _data.ch[5],
          _data.ch[6],
          _data.ch[7]
        );
        logger->debug(buffer);
      #endif
    }
  }


  void setNewData(TDATA *data) {

  }

/*
  void Receiver::write(TDATA *data) {

  }
*/

  void Receiver::write(void) {
    /*
    memcpy(sbus_data.ch, _data.ch, sizeof(sbus_data.ch));;
    sbus_data.failsafe = _data.failsafe;
    sbus_data.lost_frame = _data.lost_frame
    */
    sbus_tx->data(sbus_data);

    #if defined(LOG_TASK_RECEIVER) || defined(LOG_TASK_ALL) 
      sprintf(buffer, "Receiver::write sbus_data CH:<%4d, %4d, %4d, %4d> AUX:<%4d, %4d, %4d, %4d>", 
        sbus_data.ch[0],
        sbus_data.ch[1],
        sbus_data.ch[2],
        sbus_data.ch[3],
        sbus_data.ch[4],
        sbus_data.ch[5],
        sbus_data.ch[6],
        sbus_data.ch[7]
      );
      logger->debug(buffer);
    #endif
    
    sbus_tx->Write();
  
    #if defined(LOG_TASK_RECEIVER) || defined(LOG_TASK_ALL)
      logger->debug("RECEIVER::WRITE");
    #endif

  }
};