#include "Receiver.h"

/** constructor **/
Receiver::Receiver(SLog *log, char *name, CoopSemaphore *taskSema, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, const char *chmap)
  : PODRacer(log, name, taskSema) {
    this->_bus = bus;
    this->_invert = invert;
    this->_txpin = txpin;
    this->_rxpin = rxpin;

    if (strlen(chmap) < 8 || strlen(chmap) > 8) {
      sprintf(buffer, "Receiver::Wrong ChannelMap settings (%s) - sizeof(%d)", chmap, sizeof(chmap));
      log->error(buffer, name );
    }
    sprintf(buffer, "ChMap => %s", chmap);
    log->debug(buffer, name);
    for (uint8_t c=0; c < strlen(chmap); c++) {
        if (chmap[c] == 'A') {
          channelMap[ROLL] = c;
          sprintf(buffer, "ChMap => ROLL\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], ROLL);
        }
        else if (chmap[c] == 'E') {
          channelMap[PITCH] = c;
          sprintf(buffer, "ChMap => PITCH\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], PITCH);
        }
        else if (chmap[c] == 'T') {
          channelMap[THRUST] = c;
          sprintf(buffer, "ChMap => THRUST\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], THRUST);
        }
        else if (chmap[c] == 'R') {
          channelMap[YAW] = c;
          sprintf(buffer, "ChMap => YAW\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], YAW);
        }
        else if (chmap[c] == 'D') {
          channelMap[ARMING] = c;
          sprintf(buffer, "ChMap => ARMING\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], ARMING);
        }
        else if (chmap[c] == '1') {
          channelMap[AUX1] = c;
          sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX1);
        }
        else if (chmap[c] == '2') {
          channelMap[AUX2] = c;
          sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX2);
        }
        else if (chmap[c] == '3') {
          channelMap[AUX3] = c;
          sprintf(buffer, "ChMap => AUX1\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX3);
        }
        else if (chmap[c] == 'H') {
          channelMap[HOVERING] = c;
          sprintf(buffer, "ChMap => HOVERING\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], HOVERING);
        }
        log->debug(buffer, name);
    }
    log->info("initialized", name);
}

/** read() - SBUS data from receiver **/
void Receiver::read(TaskData *data, uint8_t preventLogging) {
  if (mock_level > 0) {
      sprintf(buffer,"READ mocking enabled. Level: %d", mock_level);
      log->once_warn(&log_once[LOG_ONCE_RECV_MOCK1], buffer, name);
      sbus_data = this->getMockedData(mock_level);
  } else {
    sbus_data = sbus_rx->data();
    for (uint8_t i=15;i--;) {
      channel_calibration[i][0] = (sbus_data.ch[i] < channel_calibration[i][0])?sbus_data.ch[i]:channel_calibration[i][0]; 
      channel_calibration[i][1] = (sbus_data.ch[i] > channel_calibration[i][1])?sbus_data.ch[i]:channel_calibration[i][1]; 
    }
  }    
  bb->clearStruct(bbd, TASK_RECEIVER);
  bbd->data.millis = millis();
  bbd->data.is_armed = false;
  bbd->data.failsafe = false;
  bbd->data.lost_frame = false;

  // know we fill our TaskData struct based on SBUS-Data which we received
  for (uint8_t i=0; i < NUMBER_CHANNELS; i++) {
    bbd->data.ch[i] = map(
                      sbus_data.ch[i], 
                      channel_calibration[i][0], 
                      channel_calibration[i][1], 
                      channel_calibration[i][2], 
                      channel_calibration[i][3]
                    );
    // if a gimbal is "around" center position, set this with a +/- noice value
    bbd->data.ch[i] = centeredValue(bbd->data.ch[i], GIMBAL_CENTER_POSITION, RECEIVER_NOISE);

    
  }
  if (bbd != nullptr) {
    if (ArmingAllowed()) {

    }
    log->data(bbd, name, "R", true, false, false, false, 0);
  }
  else {
    if (this->log_once[0]==0) {
      log->error("Receiver::read() - no SBUS-Data available", name);
      this->log_once[LOG_ONCE_RECV_ERROR]=1;
    }
  }
}

/** write() - SBUS data to receiver**/
void Receiver::write(TaskData *data, uint8_t preventLogging) {

}

TaskData *Receiver::getTaskData() {
  return bbd;
}


/** check if channel ch is in center positioin **/
bool Receiver::isGimbalCentered(uint8_t ch, bool useRange) {
  bool rc = false;
  uint8_t range (useRange ? CENTER_RANGE : 0);
  rc = isInInterval(getCurrentChannelValue(ch), GIMBAL_CENTER_POSITION, range);
  //sprintf(buffer,"ch(%d), v:%d, cRange:%d, range:%d, isCentered:%d", ch, getData(ch), GIMBAL_CENTER_POSITION, range, rc );
  //Serial.println(buffer);        
  if (rc) {
    return true;
  }
  return false;
}

/** check if ch is at MIN. If useRange is true, use a range from MIN to MIN+range **/
bool Receiver::isGimbalMin(uint8_t ch, bool useRange) {
  uint8_t range (useRange ? CENTER_RANGE : 0);
  if (isInInterval(getCurrentChannelValue(ch), GIMBAL_MIN, range)) {
    return true;
  }
  return false;
}

/** check if ch is at MAX. If useRange is true, use a range from MAX to MAX-range **/
bool Receiver::isGimbalMax(uint8_t ch, bool useRange) {
  uint8_t range (useRange ? CENTER_RANGE : 0);
  if (isInInterval(getCurrentChannelValue(ch), GIMBAL_MAX, range)) {
    return true;
  }
  return false;
}

/** check if arming is possible. Behaviour: method checks if an re-arming is possible or not. Pilot changed from ARMING to DISARM and try know to ARM again  **/
bool Receiver::ArmingAllowed() {
  // ------------------------------------------------------------------------
  // initial armingMask = 0b0000 0000
  // Arming allowed if 0b0000 0001      // arming-switch is off
  //
  //  0b0000 0000
  //    |||| ||||____ State ArmingSwitchON  1=OFF, 0=ON
  //    |||| |||_____ State of Gimbals,     1=if not centered, 0=if centered
  //    |||| ||______ State of HOVERPOTI,   1=if not MIN, 0=if MIN
  //    |||| |_______ unused
  //    ||||_________ State gimbal ROLL,    1=if not centered, 0=if centered
  //    |||__________ State gimbal PITCH,   1=if not centered, 0=if centered
  //    ||___________ State gimbal YAW      1=if not centered, 0=if centered
  //    |____________ State gimbal THRUST   1=if not centered, 0=if centered
  // ------------------------------------------------------------------------

  // Szenario 1 : pilot try to arm (current status is disarmed and want to re-arm again)
  //              Problems: gimbal or poti is not centered, arming is not allowed, preventArming is true for saftey issues
  //
  // Szenario 2 : PODRacer is armed, ignore states
  //
  //

  if (!isArmingStateON()) {    // ---------- arm switch = OFF (Szenario 1) ------------
    armingMask = 0b00001111;   // - arming is definitly not possible
    _sticksInArmingPos = sticksInArmingPos(&armingMask);
  }
  else {  
    _sticksInArmingPos = false;
    bitSet(armingMask, 0);    // ---- ---0
  }

  uint8_t armState = (armingMask & armingOKMask);
  if ( armState == armingOKMask) {
    _isPreventArming = false;
    bbd->data.is_armed = true;
    bbd->data.updated = true;
    log->once_info(&log_once[LOG_ONCE_RECV_ARM], "*****\tPODRacer - ARMED\t*******");
  }
  else {
    if (armingMask & 0x02 == 0x02) { // bit is set
      _isPreventArming = false;
    }
    else {
      _isPreventArming = true;    // bit not set
      log->once_info(&log_once[LOG_ONCE_RECV_ARM], "!!!!!\t PREVENT-ARMING\t!!!!!");
    }

  }

      // if ( armState == armingOKMask) {
      //   #if defined(LOG_TASK_RECEIVER)
      //     log->printBinary("armState =>", _tname, armState, false);
      //     log->print(" - PODRacer ARMED", true);
      //   #endif
      //   _isPreventArming = false;
      //   bbd.data.isArmed = true;
      //   _log_once_recv_disarmed = true;
      // }
      // else {
      //   #if defined(LOG_TASK_RECEIVER)
      //     logger->printBinary("armState =>", _tname, armState, false);
      //     logger->print(" - PODRacer DISARMED", true);
      //   #endif
      //   if (armingMask & 0x02 == 0x02) { // bit is set
      //     _isPreventArming = false;
      //   }
      //   else {
      //     _isPreventArming = true;    // bit not set
      //     if (_log_once_recv_disarmed) {
      //       logger->printBinary("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nPREVENT ARMING => ", _tname, armState);
      //       _log_once_recv_disarmed = false;
     //     }
      //   }
      // }

  // if preventArming is false, than arming is allowed
  return !_isPreventArming;
}

/** set mask due to gimbal and poti position, return true if they are in an arming position  or false if not, set bit mask **/
bool Receiver::sticksInArmingPos(uint8_t *mask) {
  bool rc = false;
  //uint8_t mask = *bitMask;

  (isGimbalMin(HOVERING))?bitClear(*mask,2):bitSet(*mask,2);        // 0b---- -x---
  (isGimbalCentered(ROLL))?bitClear(*mask,4):bitSet(*mask,4);        // 0b---x -----
  (isGimbalCentered(PITCH))?bitClear(*mask,5):bitSet(*mask,5);       // 0b--x- -----
  (isGimbalCentered(YAW))?bitClear(*mask,6):bitSet(*mask,6);         // 0b-x-- -----
  (isGimbalMin(THRUST))?bitClear(*mask,7):bitSet(*mask,7);           // 0bx--- -----

  if ( (*mask | 0b00001011) == 0b00001011) {         // if one (or more) of the bits 7-4 or bit2 is set than arming not possible due to gimbal/poti
    rc = true;
  }
  log->printBinary("ArmingMask =>", name, *mask);
  //*bitMask = mask;
  return rc;
}

/******************************************************************************************************/
/* MOCKing - data for this class
/******************************************************************************************************/
/** create mocking data **/
bfs::SbusData Receiver::getMockedData(uint8_t mode) {
  if (this->log_once[LOG_ONCE_RECV_MOCK2]==0) {
      log->warn ("MOCK - create new TaskData-Object", this->name);
      this->log_once[LOG_ONCE_RECV_MOCK2] = 1;
      bbd = new TaskData();
      // Armed, hovering
      // this is mock-mode 1
      sbus_data.failsafe = false;
      sbus_data.lost_frame = false;
      sbus_data.ch[ROLL] = GIMBAL_CENTER_POSITION;
      sbus_data.ch[PITCH] = GIMBAL_CENTER_POSITION;
      sbus_data.ch[YAW] = GIMBAL_CENTER_POSITION;
      sbus_data.ch[THRUST] = GIMBAL_MIN;
      sbus_data.ch[HOVERING] = GIMBAL_CENTER_POSITION;
      sbus_data.ch[ARMING] = GIMBAL_MAX;
      switch (mode) {
        case 2: {
          // preventArming, based on THRUST not MIN
          sbus_data.ch[THRUST] = GIMBAL_MAX;
          break;
        }

        default : {
          sprintf(buffer, "MOCK - unknown mode (%d)", mode);
          log->warn(buffer, this->name);
        }
      }
    }
  return sbus_data;
}
