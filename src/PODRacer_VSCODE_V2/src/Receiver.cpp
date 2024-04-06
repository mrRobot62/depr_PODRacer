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
        else if (chmap[c] == '2') {
          channelMap[AUX2] = c;
          sprintf(buffer, "ChMap => AUX2\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX2);
        }
        else if (chmap[c] == '3') {
          channelMap[AUX3] = c;
          sprintf(buffer, "ChMap => AUX3\t\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], AUX3);
        }
        else if (chmap[c] == 'H') {
          channelMap[HOVERING] = c;
          sprintf(buffer, "ChMap => HOVERING\tCH(%2d) Map('%1c'), ToPos(%2d)", c, chmap[c], HOVERING);
        }
        log->debug(buffer, name);
    }
    for (uint8_t i=0; i < NUMBER_CHANNELS; i++) {
      channel_calibration[i][CC_SBUS_CPO] = (uint16_t)(channel_calibration[i][CC_SBUS_MAX] - channel_calibration[i][CC_SBUS_MIN]) / 2 + channel_calibration[i][CC_SBUS_MIN];
    }
    bbd = new TaskData();
    log->info("initialized", name);
}

/** read() - SBUS data from receiver **/
void Receiver::read(TaskData *data, uint8_t preventLogging) {
  bb->clearStruct(bbd, TASK_RECEIVER);
  bbd->data.start_millis = millis();
  if (mock_level > 0) {
      sprintf(buffer,"MOCK-SBUS: READ mocking enabled. Level (%0d)", mock_level);
      log->once_warn(&log_once_mask, LOG_ONCE_RECV_MOCK2, buffer, name);
      sbus_data = this->getMockedData(mock_level);
  } else {
    sbus_data = sbus_rx->data();
    // create REAL SBUS MIN/MAX Values for a better calibration
    for (uint8_t i=15;i--;) {
      channel_calibration[i][CC_SBUS_MIN] = (sbus_data.ch[i] < channel_calibration[i][CC_SBUS_MIN])?channel_calibration[i][CC_SBUS_MIN]:sbus_data.ch[i]; 
      channel_calibration[i][CC_GIMB_MAX] = (sbus_data.ch[i] > channel_calibration[i][CC_SBUS_MAX])?channel_calibration[i][CC_SBUS_MAX]:sbus_data.ch[i]; 
    }
  }    

  bbd->data.is_armed = false;
  bbd->data.failsafe = false;
  bbd->data.lost_frame = false;

  // know we fill our TaskData struct based on SBUS-Data which we received
  for (uint8_t i=0; i < NUMBER_CHANNELS; i++) {
    bbd->data.ch[i] = map(
                      sbus_data.ch[i],                          // this values should be mapped
                      channel_calibration[i][CC_SBUS_MIN],      // between MIN
                      channel_calibration[i][CC_SBUS_MAX],      // and MAX
                      channel_calibration[i][CC_GIMB_MIN],      // to 1000
                      channel_calibration[i][CC_GIMB_MAX]       // and 2000
                    );
    // if a gimbal is "around" center position, set this with a +/- noice value
    bbd->data.ch[i] = centeredValue(bbd->data.ch[i], GIMBAL_CENTER_POSITION, RECEIVER_NOISE);
  }
  
  log->once_data(&log_once_mask, LOG_ONCE_MOCK3_BIT, bbd, name, "RAW1",true);

  if (bbd != nullptr) {
    if (ArmingAllowed()) {
        bbd->data.end_millis = millis();
        bbd->data.is_armed = true;
        bbd->data.updated = true;
        log->once_data(&log_once_mask, LOG_ONCE_RECV_MOCK1, bbd, name, "RAW2", true);
        log->once_info(&log_once_mask, LOG_ONCE_RECV_ARM, "PODRacer ARMED", name);
        log->data(bbd, name, "RD", true, false, false, false);
    } else {
          bbd->data.end_millis = millis();
        log->once_info(&log_once_mask, LOG_ONCE_RECV_DISARM, "PODRacer DIS-ARMED", name);
    }
  }
  else {
      bbd->data.end_millis = millis();
      log->once_error(&log_once_mask, LOG_ONCE_ERROR0_BIT, "Receiver::read() - no SBUS-Data available", name);
  }
  
}

/** write() - SBUS data to receiver**/
void Receiver::write(TaskData *data, uint8_t preventLogging) {
  data->data.start_millis = millis();

  delay(10);
  data->data.end_millis = millis();
  log->data(data, name, "WR", true);
}

TaskData *Receiver::getTaskData() {
  return bbd;
}
/** check if channel ch is in center positioin **/
bool Receiver::isGimbalCentered(uint8_t ch, bool useRange) {
  bool rc = false;
  uint8_t range (useRange ? CENTER_RANGE : 0);
  rc = isInInterval(getCurrentChannelValue(ch), GIMBAL_CENTER_POSITION, range);
  
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

void getPreventArmingBits(uint8_t *mask, char* buf) {
  // if bit set, ok, if 0 than set HRPYTπ
  (bitRead(*mask, 3))?buf[4] = '-':buf[4] = 'H';
  (bitRead(*mask, 4))?buf[3] = '-':buf[3] = 'R';
  (bitRead(*mask, 5))?buf[2] = '-':buf[2] = 'P';
  (bitRead(*mask, 6))?buf[1] = '-':buf[1] = 'Y';
  (bitRead(*mask, 7))?buf[0] = '-':buf[0] = 'T';
}

/** check if arming is possible. Behaviour: method checks if an re-arming is possible or not. Pilot changed from ARMING to DISARM and try know to ARM again  **/
bool Receiver::ArmingAllowed() {
  // ------------------------------------------------------------------------
  // initial armingMask = 0b0000 1111   // no arming possible
  // Arming allowed if 0b0000 0001      // arming-switch is off, bit 1-3 = 0, bit 4-7 don't matter
  //
  //  0b0000 0000
  //    |||| ||||____ 0-State ArmingSwitchON  0=OFF, 1=ON
  //    |||| |||_____ 1-State of Gimbals,     0=if not centered, 1=if centered
  //    |||| ||______ 2-unused
  //    |||| |_______ 3-State of HOVERPOTI,   0=if not MIN, 1=if MIN
  //    ||||_________ 4-State gimbal ROLL,    0=if not centered, 1=if centered
  //    |||__________ 5-State gimbal PITCH,   0=if not centered, 1=if centered
  //    ||___________ 6-State gimbal YAW      0=if not centered, 1=if centered
  //    |____________ 7-State gimbal THRUST   0=if not centered, 1=if centered
  // ------------------------------------------------------------------------

  // Szenario 1 : pilot try to arm (current status is disarmed and want to re-arm again)
  //              Problems: gimbal or poti is not centered, arming is not allowed, preventArming is true for saftey issues
  //
  // Szenario 2 : PODRacer is armed, ignore states
  //
  //
  _isPreventArming == false;
  if (isArmingChannelON() == ARMING_OFF) {    // ---------- arm switch = OFF (Szenario 1) ------------
    // ---------------------------------------------------------------------------
    // Armswitch OFF, check if arming is possible
    // ---------------------------------------------------------------------------
    armingMask = 0b00000000;   // - arming is definitly not possible
    _sticksInArmingPos = sticksInArmingPos(&armingMask);

    // gimbals and poti are in position to allow arming
    // preventArming is false,
    if ((armingMask & armingOKMask) == armingOKMask ) {
      _isPreventArming = false;
      bbd->data.is_armed = false;
      bbd->data.updated = true;
      log->once_info(&log_once_mask, LOG_ONCE_RECV_ARM, "----- ARMING - allowed -----", name);
      resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_ARM);
      resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_DISARM);
      resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_PREVENT);
      resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_ARMING_PATTERN);
    } else {
      // ---------------------------------------------------------------------------
      // prevent arming
      // ---------------------------------------------------------------------------
      // one or more GIMBALs or POTI are not in the right position
      // prevent arming
      _isPreventArming = true;  
      char pbc[5] = {0};    // hold char for gimbals which prevents arming
      getPreventArmingBits(&armingMask, pbc);
      sprintf(buffer, "!!!!!  PREVENT-ARMING  !!!!! PrevArmFlag: %d Mask [%s]", _isPreventArming, pbc);
      log->once_info(&log_once_mask, LOG_ONCE_RECV_PREVENT,buffer, name);
      // if preventArming is false, than arming is allowed
    }
    return _isPreventArming;
  }
  else if (_isPreventArming == false) {  
    // ---------------------------------------------------------------------------
    // Armswitch ON, PODRacer can fly
    // ---------------------------------------------------------------------------
    _sticksInArmingPos = true;
    _isPreventArming = false;
    bbd->data.is_armed = true;
    bbd->data.updated = true;
    log->once_info(&log_once_mask, LOG_ONCE_RECV_ARM, "*****\tPODRacer - ARMED\t*******", name);

    resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_ARM);
    resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_DISARM);
    resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_PREVENT);
    resetLogOnceBit(&log_once_mask, LOG_ONCE_RECV_ARMING_PATTERN);
    return !_isPreventArming;
  }
  return _isPreventArming;
}

/** set mask due to gimbal and poti position, return true if they are in an arming position  or false if not, set bit mask **/
bool Receiver::sticksInArmingPos(uint8_t *mask) {
  bool rc = false;
  // if gimbal is in centerpos set bit if not clear bit
  // if poti is min set bit, if not clear bit
  (isGimbalMin(HOVERING))   ?bitSet(*mask,3): bitClear(*mask,3);          // 0b---- x----
  (isGimbalCentered(ROLL))  ?bitSet(*mask,4): bitClear(*mask,4);          // 0b---x -----
  (isGimbalCentered(PITCH)) ?bitSet(*mask,5): bitClear(*mask,5);          // 0b--x- -----
  (isGimbalCentered(YAW))   ?bitSet(*mask,6): bitClear(*mask,6);          // 0b-x-- -----
  (isGimbalMin(THRUST))     ?bitSet(*mask,7): bitClear(*mask,7);          // 0bx--- -----

  bitSet(*mask, 2);                             // set unused bit
  if ( (*mask & 0b11111000) == 0b11111000) {    // if one (or more) of the bits 7-3 is not set than arming not possible due to gimbal/poti
    rc = true;
    bitSet(*mask, 1);                           // set bit 1
  }
  else {
    bitClear(*mask, 1);                         // clear bit 1
  }
  //log->printBinary("(ArmingPos=11111---): ", name, (*mask & 0b11111000));
  log->once_binary(&log_once_mask, LOG_ONCE_RECV_ARMING_PATTERN, "(ArmingPos=11111---): ", name, (*mask & 0b11111000));
  return rc;
}

/******************************************************************************************************/
/* MOCKing - data for this class
/******************************************************************************************************/
/** create mocking data **/
bfs::SbusData Receiver::getMockedData(uint8_t mode) {
  log->once_warn (&log_once_mask, LOG_ONCE_MOCK1_BIT, "MOCK-SBUS: create new SBUS-DATA", this->name);
  // Armed, hovering, little thrust
  // this is mock-mode 1
  sbus_data.failsafe = false;
  sbus_data.lost_frame = false;
  sbus_data.ch[ROLL] =    channel_calibration[ROLL][CC_SBUS_CPO];       // CenterPos
  sbus_data.ch[PITCH] =   channel_calibration[PITCH][CC_SBUS_CPO];      // CenterPos
  sbus_data.ch[YAW] =     channel_calibration[YAW][CC_SBUS_CPO];        // CenterPos
  sbus_data.ch[THRUST] =  250;                                          // little Thrust
  sbus_data.ch[HOVERING]= channel_calibration[HOVERING][CC_SBUS_CPO];   // Poti - mid position
  sbus_data.ch[AUX2] =  channel_calibration[AUX2][CC_SBUS_MIN];         // MIN
  sbus_data.ch[AUX3] =  channel_calibration[AUX3][CC_SBUS_MIN];         // MIN
  switch (mode) {
    case 1: {  
      sbus_data.ch[ARMING] =  channel_calibration[ARMING][CC_SBUS_MAX];     // ARMED
      sbus_data.ch[HOVERING]= channel_calibration[HOVERING][CC_SBUS_CPO] + 200;
      break;}
    case 2: {
      // disarmed, but arming should be possible
      sbus_data.ch[HOVERING]= channel_calibration[HOVERING][CC_SBUS_MIN];
      sbus_data.ch[ARMING] =  channel_calibration[ARMING][CC_SBUS_MIN]; // DISARMED
      break;
    }
    case 3: {
      // preventArming, based on Gimbal / or Poti
      sbus_data.ch[THRUST] =  channel_calibration[THRUST][CC_SBUS_MAX];
      //sbus_data.ch[ROLL] =  channel_calibration[ROLL][CC_SBUS_MIN];
      //sbus_data.ch[PITCH] =  channel_calibration[PITCH][CC_SBUS_MIN];
      //sbus_data.ch[YAW] =  channel_calibration[YAW][CC_SBUS_MAX];
      sbus_data.ch[HOVERING]= channel_calibration[HOVERING][CC_SBUS_MIN];
      sbus_data.ch[ARMING] =  channel_calibration[ARMING][CC_SBUS_MIN]; // DISARMED
      break;
    }

    default : {
      sprintf(buffer, "MOCK - unknown mode (%d)", mode);
      log->warn(buffer, this->name);
    }
  }
  sprintf(buffer, "MOCK-SBUS: R[%d], P[%d], Y[%d], T[%d], H[%d], ARM[%d], AUX2-3:[%d][%d]",
    sbus_data.ch[ROLL], sbus_data.ch[PITCH], sbus_data.ch[YAW],
    sbus_data.ch[THRUST], sbus_data.ch[HOVERING], sbus_data.ch[ARMING],
    sbus_data.ch[AUX2],  sbus_data.ch[AUX3]
    );

  log->once_info (&log_once_mask, LOG_ONCE_MOCK2_BIT, buffer, name);

  return sbus_data;
}
