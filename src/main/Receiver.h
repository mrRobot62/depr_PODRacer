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
      Receiver(uint8_t taskID, SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, Blackbox *bb=nullptr, const char *chmap="AETRD23H");

      bool begin(void);

      /** read data from receiver and update internal SDATA struct **/
      void update(void);

      /** update internal SbusData struct. This struct will be written back to receiver with write() **/
      void setNewData(uint8_t ch, uint16_t value);

      /** send data to receiver **/
      void write(void);


      uint16_t getRoll() {
        
      }

      /** return channel data from last update **/
      uint16_t getData(int8_t ch) {
        ch = constrain(ch, 0, NUMBER_CHANNELS-1 );
        return _bbd.data.ch[ch];
      }

      /** return failsafe from last update **/
      bool isFailSave() {
        return _bbd.data.failsafe;
      }

      /** return lostframe from last update **/
      bool isLostFrame() {
        return _bbd.data.lost_frame;
      }

      /** **/
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


      /** **/
      bool isGimbalMin(uint8_t ch, bool useRange=true) {
        uint8_t range (useRange ? CENTER_RANGE : 0);
        if (isInInterval(getData(ch), GIMBAL_MIN, 0, range)) {
          return true;
        }
        return false;
      }

      /** **/
      bool isGimbalMax(uint8_t ch, bool useRange=true) {
        uint8_t range (useRange ? CENTER_RANGE : 0);
        if (isInInterval(getData(ch), GIMBAL_MAX, range, 0)) {
          return true;
        }
        return false;
      }

    private:
      HardwareSerial *_bus;
      uint8_t _txpin, _rxpin;
      bool _invert;

      bfs::SbusRx *sbus_rx;
      bfs::SbusTx *sbus_tx;
      bfs::SbusData sbus_data, last_data;

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