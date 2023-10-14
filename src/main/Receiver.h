#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include <sbus.h>
#include "Task.h"
#include "constants.h"

#define MIN_CH_VAL 1000
#define MAX_CH_VAL 2000
#define CENTER_CH_VAL 1500

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
      Receiver(SLog *log, HardwareSerial *bus, uint8_t rxpin, uint8_t txpin, bool invert, const char *chmap="AETR");

      bool begin(void);

      /** read data from receiver and update internal SDATA struct **/
      void update(void);

      /** update internal SbusData struct. This struct will be written back to receiver with write() **/
      void setNewData(TDATA *data);

      /** send data to receiver **/
      void write(void);


      uint16_t getRoll() {
        
      }

      /** return channel data from last update **/
      uint16_t getData(int8_t ch) {
        ch = constrain(ch, 0, _data.NUM_CH-1 );
        return _data.ch[ch];
      }

      /** return failsafe from last update **/
      bool isFailSave() {
        return _data.failsafe;
      }

      /** return lostframe from last update **/
      bool isLostFrame() {
        return _data.lost_frame;
      }


    private:
      HardwareSerial *_bus;
      uint8_t _txpin, _rxpin;
      bool _invert;

      bfs::SbusRx *sbus_rx;
      bfs::SbusTx *sbus_tx;
      bfs::SbusData sbus_data;

      // mapping Roll/Pitch/Throttle/Yaw axis to channels
      // default : ch0 = roll, ch1 = pitch, ch2 = throttle, ch3=yaw
      // ch4 and others = aux
      //
      uint8_t channelMap[4] = {0,1,2,3};
      uint16_t channel_calibration [16][4] = {
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