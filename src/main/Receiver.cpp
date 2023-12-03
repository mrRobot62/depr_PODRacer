#include "Arduino.h"
#include "Receiver.h"


  Receiver::Receiver(uint8_t taskID, SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, Blackbox *bb, const char *chmap) : TaskAbstract(taskID, log, bb)  {
    _bus = bus;
    _invert = invert;
    _txpin = txpin;
    _rxpin = rxpin;
    _tname = "RECV";

    if (strlen(chmap) < 8 || strlen(chmap) > 8) {
      sprintf(buffer, "Receiver::Wrong ChannelMap settings (%s) - sizeof(%d)", chmap, sizeof(chmap));
      logger->error(buffer, _tname );
    }
    logger->info("ChMap => ", _tname, false) ; logger->print(chmap, true);
    for (uint8_t c=0; c < strlen(chmap); c++) {
        if (chmap[c] == 'A') {
          channelMap[ROLL] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => ROLL\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], ROLL);
          #endif
        }
        else if (chmap[c] == 'E') {
          channelMap[PITCH] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => PITCH\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], PITCH);
          #endif
        }
        else if (chmap[c] == 'T') {
          channelMap[THRUST] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => THRUST\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], THRUST);
          #endif
        }
        else if (chmap[c] == 'R') {
          channelMap[YAW] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => YAW\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], YAW);
          #endif
        }
        else if (chmap[c] == 'D') {
          channelMap[ARMING] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => ARMING\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], ARMING);
          #endif
        }
        else if (chmap[c] == '1') {
          channelMap[AUX1] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX1);
          #endif
        }
        else if (chmap[c] == '2') {
          channelMap[AUX2] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX2);
          #endif
        }
        else if (chmap[c] == '3') {
          channelMap[AUX3] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX3);
          #endif
        }
        else if (chmap[c] == 'H') {
          channelMap[HOVERING] = c;
          #if defined(LOG_TASK_RECEIVER)
            sprintf(buffer, "ChMap => HOVERING\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], HOVERING);
          #endif
        }
        #if defined(LOG_TASK_RECEIVER)
          logger->info(buffer, _tname);
        #endif

    }
    logger->info("initialized", _tname);
  }

  bool Receiver::begin(void) {
    bool rc = false;
    sbus_rx = new bfs::SbusRx(_bus, _rxpin, _txpin, _invert);
    sbus_tx = new bfs::SbusTx(_bus, _rxpin, _txpin, _invert);
    if (sbus_rx == NULL || sbus_tx == NULL) {
      logger->error("can't create a SBus rx/tx object", _tname);
      rc = false;
      setError(getID(), 0x02);
      return rc;
    }
    //Serial.println("RECV before sbus_rx/tx.Begin()");
    sbus_rx->Begin();   // pin from SBUS-RECEIVER -> ESP32(RX)
    sbus_tx->Begin();   // pin from ESP32(TX) -> FlightController
    _bbd.data.isArmed = false;
    //Serial.println("RECV after sbus_rx/tx.Begin()");
    armingMask = 0b00000000;
    _armSwitchOn = false;
/*
    for (uint8_t i=15;i--;) {
      channel_calibration[i][0] = GIMBAL_MAX;      // set MIN to a high number
      channel_calibration[i][1] = GIMBAL_MIN;      // set MAX to a low number
    }
*/
    sprintf(buffer, "ready");
    logger->info(buffer, _tname);
    rc = true;
    _log_once_recv_disarmed = true;
    return rc;
  }

  bool Receiver::sticksInArmingPos() {
    _sticksInArmingPos = false;
   if (
      isGimbalCentered(ROLL) &&      //  centered
      isGimbalCentered(PITCH) &&     //  centered
      isGimbalCentered(YAW) &&       //  centered
      isGimbalMin(THRUST) &&         //  MIN   
      isGimbalMin(HOVERING)          //  MIN
    ) {
        _sticksInArmingPos = true;
    }
    return _sticksInArmingPos;
  }

  void Receiver::update(void) {

    if (sbus_rx->Read()) {
      // read data from sbus-receiver
      sbus_data = sbus_rx->data();
      for (uint8_t i=15;i--;) {
        channel_calibration[i][0] = (sbus_data.ch[i] < channel_calibration[i][0])?sbus_data.ch[i]:channel_calibration[i][0]; 
        channel_calibration[i][1] = (sbus_data.ch[i] > channel_calibration[i][1])?sbus_data.ch[i]:channel_calibration[i][1]; 
        // sprintf(buffer, "%2d ch_calibration: %04d, %04d, %04d, %04d", 
        //   i,
        //   channel_calibration[i][0],
        //   channel_calibration[i][1],
        //   channel_calibration[i][2],
        //   channel_calibration[i][3]
        // );
        // logger->info(buffer);
      }
      #if defined(LOG_TASK_RECEIVER_RRAW)
        sprintf(buffer,"RAW1:%4d, RAW:% 4d, RAW3:%4d, RAW4:%4d, RAW5:%4d, RAW6:%4d, RAW7:%4d, RAW8:%4d",
          sbus_data.ch[0],
          sbus_data.ch[1],
          sbus_data.ch[2],
          sbus_data.ch[3],
          sbus_data.ch[4],
          sbus_data.ch[5],        
          sbus_data.ch[6],        
          sbus_data.ch[7]
        );
        logger->info(buffer, "RECVR");      
      #endif

      uint16_t v1,v2;
      _blackbox->clearStruct(&_bbd, TASK_RECEIVER);

      // map sbus data to internal _bbd struct
      
      _bbd.data.millis = millis();
      for (uint8_t i=0; i < NUMBER_CHANNELS; i++) {
        _bbd.data.ch[i] = map(
                          sbus_data.ch[i], 
                          channel_calibration[i][0], 
                          channel_calibration[i][1], 
                          channel_calibration[i][2], 
                          channel_calibration[i][3]
                        );
        // if a gimbal is "around" center position, set this with a +/- noice value
        _bbd.data.ch[i] = centeredValue(_bbd.data.ch[i], GIMBAL_CENTER_POSITION, RECEIVER_NOISE);
      }
      // only a local state for arming, this is later used to 
      // check some security mechanism and calculate a result if arming is allowed
      _armSwitchOn = (_bbd.data.ch[ARMING] > ARMING_VALUE)?true:false;
      #if defined(TEST_NO_TRANSMITTER_USED)
        _armSwitchOn = true;
      #endif
      // 
      _bbd.data.failsafe = sbus_data.failsafe;
      _bbd.data.lost_frame = sbus_data.lost_frame;
      _bbd.data.isArmed = false;    // for safety set to disarmed. This variable is set to true due to several security meachanisms
      // ------------------------------------------------------------------------
      // initial armingMask = 0b0000 0000
      // arming only allowed if 0b0000 1010
      //
      //  0b0000 0000
      //         |||⎿__ 1 if arm switch is OFF, reset if arm switch is ON
      //.        ||⎿___ 1 if one Gimbal or PORTI is not centered
      //         |⎿____ unused
      //         ⎿_____ 1 if arm switch is ON
      // ------------------------------------------------------------------------
      if (_armSwitchOn == false) {    // ---------- arm switch = OFF ------------
        bitSet(armingMask, 0);        // 0b---- ---1
        bitClear(armingMask, 3);      // 0b---- 0---

        if (sticksInArmingPos()) {
          bitSet(armingMask,1);       // 0b---- --1-
        }
        else {
          bitClear(armingMask,1);     // 0b---- --0-
          _isPreventArming = true;
        }
      }
      else {  //------ arm switch = ON --------
        bitSet(armingMask, 3);        // 0b---- 1---
        bitClear(armingMask, 0);      // 0b---- ---0
      }

      //logger->printBinary("armingMask =>", _tname, armingMask);

      uint8_t armState = (armingMask & armingOKMask);
      if ( armState == armingOKMask) {
        #if defined(LOG_TASK_RECEIVER)
          logger->printBinary("armState =>", _tname, armState, false);
          logger->print(" - PODRacer ARMED", true);
        #endif
        _isPreventArming = false;
        _bbd.data.isArmed = true;
        _log_once_recv_disarmed = true;
      }
      else {
        #if defined(LOG_TASK_RECEIVER)
          logger->printBinary("armState =>", _tname, armState, false);
          logger->print(" - PODRacer DISARMED", true);
        #endif
        if (armingMask & 0x02 == 0x02) { // bit is set
          _isPreventArming = false;
        }
        else {
          _isPreventArming = true;    // bit not set
          if (_log_once_recv_disarmed) {
            logger->printBinary("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nPREVENT ARMING => ", _tname, armState);
            _log_once_recv_disarmed = false;
          }
        }
      }
      _bbd.data.updated = true;
      #if defined(TEST_NO_TRANSMITTER_USED)
        _bbd.data.isArmed = true;
      #endif
      #if defined(LOG_VISUALIZER)
        if (_bbd.data.isArmed) {
            send2Visualizer(_tname, "RD", &_bbd);
        }
      #endif

      #if defined(LOG_TASK_RECEIVER_R) && defined(USE_SERIAL_PLOTTER)
        sprintf(buffer,"RCH1:%4d, RCH2:%4d, RCH3:%4d, RCH4:%4d, RCH5:%4d, RCH6:%4d, RCH7:%4d, RCH8:%4d",
          _bbd.data.ch[0],
          _bbd.data.ch[1],
          _bbd.data.ch[2],
          _bbd.data.ch[3],
          _bbd.data.ch[4],
          _bbd.data.ch[5],        
          _bbd.data.ch[6],        
          _bbd.data.ch[7]
        );
        logger->info(buffer, "RECVR");
      #endif
    }
    else {
      setError(getID(),0x03);
    }

    // if function to the end, assumption is, that internal data struct was updated
    setUpdateFlag();
    resetError();
  }

  void Receiver::write(void) {
    logger->warn("do not use this write-function");
  }

  void Receiver::write(BBD *data) {
    
    // copy data.ch array to sbus_data.ch array
    memcpy(write_data.ch, data->data.ch, sizeof(sbus_data.ch));;
    for (uint8_t i=15;i--;) {
      write_data.ch[i] = map(write_data.ch[i],
        channel_calibration[i][2],  // mapping to a range from 1000 to 2000
        channel_calibration[i][3],  // mapping to a range from 1000 to 2000
        channel_calibration[i][0],  // mapping to a range from ~170 to 1800
        channel_calibration[i][1]   // mapping to a range from ~170 to 1800
      );
    }
    sbus_tx->data(write_data);
    #if defined(LOG_TASK_RECEIVER_W)
      sprintf(buffer,"WCH1:%4d, WCH2:%4d, WCH3:%4d, WCH4:%4d, WCH5:%4d, WCH6:%4d, WCH7:%4d, WCH8:%4d",
        write_data.ch[0],
        write_data.ch[1],
        write_data.ch[2],
        write_data.ch[3],
        write_data.ch[4],
        write_data.ch[5],
        write_data.ch[6],
        write_data.ch[7]
      );            
      logger->info(buffer, "RECVW");
    #endif
      #if defined(LOG_VISUALIZER)
        send2VisualizerSBUS(_tname, "WR", &write_data);
      #endif
    sbus_tx->Write();
  }
