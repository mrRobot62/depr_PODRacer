#include "Receiver.h"


  Receiver::Receiver(uint8_t taskID, SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, const char *chmap) : TaskAbstract(taskID, log)  {
    _bus = bus;
    _invert = invert;
    _txpin = txpin;
    _rxpin = rxpin;

    if (strlen(chmap) < 8 || strlen(chmap) > 8) {
      sprintf(buffer, "Wrong ChannelMap settings (%s) - sizeof(%d)", chmap, sizeof(chmap));
      logger->error(buffer);
    }
    for (uint8_t c=0; c < strlen(chmap); c++) {
        if (chmap[c] == 'A') {
          channelMap[ROLL] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], ROLL);
          #endif
        }
        else if (chmap[c] == 'E') {
          channelMap[PITCH] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], PITCH);
          #endif
        }
        else if (chmap[c] == 'T') {
          channelMap[THROTTLE] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], THROTTLE);
          #endif
        }
        else if (chmap[c] == 'R') {
          channelMap[YAW] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], YAW);
          #endif
        }
        else if (chmap[c] == '1') {
          channelMap[AUX1] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX1);
          #endif
        }
        else if (chmap[c] == '2') {
          channelMap[AUX2] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX2);
          #endif
        }
        else if (chmap[c] == '3') {
          channelMap[AUX3] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX3);
          #endif
        }
        else if (chmap[c] == 'H') {
          channelMap[HOVER] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "CH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], HOVER);
          #endif
        }
        #if defined(LOG_TASK_RECEIVER)
          logger->info(buffer);
        #endif

    }
    logger->info("Channel mapping: ", false) ; logger->info(chmap);
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
    logger->info("Receiver ready");
    return rc;
  }


  void Receiver::update(void) {

    if (sbus_rx->Read()) {
      sbus_data = sbus_rx->data();
      uint16_t v1,v2;
      for (uint8_t i=0; i < _data.NUM_CH;i++) {
        _data.ch[i] = map(
                          sbus_data.ch[i], 
                          channel_calibration[i][0], 
                          channel_calibration[i][1], 
                          channel_calibration[i][2], 
                          channel_calibration[i][3]
                        );
        _data.ch[i] = centeredValue(_data.ch[i], GIMBAL_CENTER_POSITION, RECEIVER_NOISE);
      }
      _data.failsafe = sbus_data.failsafe;
      _data.lost_frame = sbus_data.lost_frame;
      _data.updated = true;

      #if defined(LOG_TASK_RECEIVER_R) && defined(USE_SERIAL_PLOTTER)
        sprintf(buffer, "READ CH1:%d, CH2:%d, CH3:%d, CH4:%d, CH5:%d, CH6:%d, CH7:%d, CH8:%d",
          _data.ch[0],
          _data.ch[1],
          _data.ch[2],
          _data.ch[3],
          _data.ch[4],
          _data.ch[5],        
          _data.ch[6],        
          _data.ch[7]     
        );
        logger->simulate(buffer);
      #endif
    }
  }


  /** add value to current sbus_data.ch[ch] value **/
  void  Receiver::setNewData(uint8_t ch, uint16_t value) {
    if ( (ch > 0) && (ch < _data.NUM_CH)) {
      sbus_data.ch[ch] = +value; 
      /** for safety - take care that value is inside gimbal-range **/
      sbus_data.ch[ch] = constrain(sbus_data.ch[ch], GIMBAL_MIN, GIMBAL_MAX);
    }
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

    #if defined(LOG_TASK_RECEIVER) || defined (USE_SERIAL_PLOTTER)

    #elif defined(LOG_TASK_RECEIVER) || defined(LOG_TASK_ALL) 
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
  }
