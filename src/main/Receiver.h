#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include <sbus.h>
#include "Task.h"
#include "constants.h"

/**

  Receiver class


  read sbus data from Receiver and can write back a sbusdata struct back to hardware-receiver

  update():
    update internal sbusdata struct with latest hardware-receiver data. This function is called via task scheduler

  setNewData()
    update the internal sbusdata strucht with new data which is send by on other task. This function is called via arbitrateTask

  write()
    write sbusdata back to receiver


  workflow
    receiverTask is called every nMS and call update()
    tasks like opticalFlowTask and others create new data which should be send to receiver
    arbitrateTask prior the incomming task data and call setNewData() to update the sbusdata struct
    arbitrateTask call write() and data is send to the hardware receiver 


**/


/*
  typedef struct data SDATA;  
  struct data {
    bool updated;
    bool failsafe;
    bool lostframe;
    static constexpr int8_t NUM_CH = 16;
    int16_t ch[NUM_CH];        
  }; 
*/

  class Receiver : public TaskAbstract {
    public:
      Receiver(uint8_t taskID, SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, Blackbox *bb=nullptr, HardwareSerial *visBus=nullptr, const char *chmap="AEHRD23T");

      /** starting the receiver **/
      bool begin(void);

      /** read data from receiver and update internal SDATA struct **/
      void update(void);

      /** send data to receiver **/
      void write(void);
      void write(BBD *data);
      /** return true, if R & P & Y=CenterPos && T & H in MIN Pos **/
      bool sticksInArmingPos();

      /** if return true, arming not possible. Used by tasks (eg BlinkTask) to set a task behaviour **/
      bool isPreventArming() {
        return _isPreventArming;
      }
      /** return channel data from last update **/
      uint16_t getData(int8_t ch) {
        ch = constrain(ch, 0, NUMBER_CHANNELS-1 );
        return _bbd.data.ch[ch];
      }

      /** return true if faile save or lost frames detected **/
      bool isEmergencyStop() {
        return isFailSave() || isLostFrame();
      }

      /** return failsafe from last update **/
      bool isFailSave() {
        return _bbd.data.failsafe;
      }

      /** return lostframe from last update **/
      bool isLostFrame() {
        return _bbd.data.lost_frame;
      }

      /** return current arming state **/
      bool isArmed() {
        #if defined(TEST_NO_TRANSMITTER_USED)
          return true;
        #endif
        return _bbd.data.isArmed;
      }

      /** DO NOT USE THIS setting - maybe it's override your real arming state !!!!! Using only for tests **/
      bool setArmed() {
        _bbd.data.isArmed = true;
      }

      /** check if channel ch is in center positioin **/
      bool isGimbalCentered(uint8_t ch, bool useRange=true) {
        bool rc = false;
        uint8_t range (useRange ? CENTER_RANGE : 0);
        rc = isInInterval(getData(ch), GIMBAL_CENTER_POSITION, range);
        //sprintf(buffer,"ch(%d), v:%d, cRange:%d, range:%d, isCentered:%d", ch, getData(ch), GIMBAL_CENTER_POSITION, range, rc );
        //Serial.println(buffer);        
        if (rc) {
          return true;
        }
        return false;
      }


      /** check if ch is at MIN. If useRange is true, use a range from MIN to MIN+range **/
      bool isGimbalMin(uint8_t ch, bool useRange=true) {
        uint8_t range (useRange ? CENTER_RANGE : 0);
        if (isInInterval(getData(ch), GIMBAL_MIN, range)) {
          return true;
        }
        return false;
      }

      /** check if ch is at MAX. If useRange is true, use a range from MAX to MAX-range **/
      bool isGimbalMax(uint8_t ch, bool useRange=true) {
        uint8_t range (useRange ? CENTER_RANGE : 0);
        if (isInInterval(getData(ch), GIMBAL_MAX, range)) {
          return true;
        }
        return false;
      }



    private:
      HardwareSerial *_bus;
      uint8_t _txpin, _rxpin;
      uint8_t armingMask;                     // 0b----XXXX, bit 0=initialBit, 1=stickBasePos, 2=free, 3=armSwitch, 4-7=0
      uint8_t armingOKMask = 0b00001010;      // b3 = arming ON (1), b2=0, b1=stickCP (1), b0=initial(0) => 1010
      bool _invert;
      bool _sticksInArmingPos = false;
      bool _isPreventArming = true;
      bool _armSwitchOn = false;
      bool _log_once_recv_disarmed;
      bfs::SbusRx *sbus_rx;
      bfs::SbusTx *sbus_tx;
      bfs::SbusData sbus_data, write_data, last_data;

      // mapping Roll/Pitch/Throttle/Yaw axis to channels
      // default : ch0 = roll, ch1 = pitch, ch2 = throttle, ch3=yaw
      // ch4 and others = aux
      //
      uint8_t channelMap[NUMBER_CHANNELS] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
      uint16_t channel_calibration [NUMBER_CHANNELS][4] = {
        // calibrations values for 16 channels
        // idx0= SBUS lowes value, idx1=SBUS highest value, idx2+3=calibrated min/max values
        {192, 1800, GIMBAL_MIN, GIMBAL_MAX},
        {192, 1794, GIMBAL_MIN, GIMBAL_MAX},
        {192, 1794, GIMBAL_MIN, GIMBAL_MAX},
        {192, 1794, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX},
        {173, 1810, GIMBAL_MIN, GIMBAL_MAX}
      };
  };

#endif