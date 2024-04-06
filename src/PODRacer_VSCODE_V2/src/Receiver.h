/*




*/
#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include "PODRacer.h"

// LOG-ONCE mechanism only for this class
// bit 8-15 are individual for classes and are define in the class header file 
#define LOG_ONCE_RECV_ARM 8
#define LOG_ONCE_RECV_DISARM 9
#define LOG_ONCE_RECV_ARMING_PATTERN 10
#define LOG_ONCE_RECV_XXYYYY 11
#define LOG_ONCE_RECV_XXYYZZ 12
#define LOG_ONCE_RECV_PREVENT 13
#define LOG_ONCE_RECV_MOCK1 14
#define LOG_ONCE_RECV_MOCK2 15


class Receiver : public PODRacer{
  public:
    Receiver(SLog *log, char *name, CoopSemaphore *taskSema, HardwareSerial *bus, uint8_t rxpin, uint8_t txping, bool invert, const char *chmap="AEHRD23T");
    // ~Receiver(){};
    bool begin(void);                                       // initialize the receiver
    void read(TaskData *data, uint8_t preventLogging=1);    // read sbus data and store result inside data struct, preventLogging=0 do logging, 1=supress logging
    void write(TaskData *data, uint8_t preventLogging=1);   // write data back to sbus receiver, preventLogging=0 do logging, 1=supress logging
    TaskData *getTaskData();

    bool sticksInArmingPos(uint8_t *mask);                  // return true, if all sticks are in arming pos R, P, Y = CenterPos, T, H = MIN_POS
    bool isPreventArming() {                                // return true, arming is not possible. Used by task to set a task behaviour
      return _isPreventArming;
    }

    /** return current value of channel 'ch' */
    uint16_t getCHData(uint8_t ch) {
      ch = constrain(ch, 0, NUMBER_CHANNELS);
      return bbd->data.ch[ch];
    }

    //void setMock(uint8_t mock_level) {this->setMock(mock_level);};

    /** check if channel ch is in center positioin **/
    bool isGimbalCentered(uint8_t ch, bool useRange=true);

    /** check if ch is at MIN. If useRange is true, use a range from MIN to MIN+range **/
    bool isGimbalMin(uint8_t ch, bool useRange=true);

    /** check if ch is at MAX. If useRange is true, use a range from MAX to MAX-range **/
    bool isGimbalMax(uint8_t ch, bool useRange=true);

    // not used by receiver
    TaskData *getMockedData(TaskData *td, uint8_t mode) {return nullptr;};
  
    // receiver use a special mocking method, because we write direcitly into the sbus_data_struact;
    bfs::SbusData getMockedData(uint8_t mode);

    inline bool IsPreventArming() {return _isPreventArming;};     // variable is set by ArmingAllowed()

    
    inline uint16_t getCurrentChannelValue(int8_t ch) {
      ch = constrain(ch, 0, NUMBER_CHANNELS-1);
      return bbd->data.ch[ch];
    }

    /** return true if faile save or lost frames detected **/
    inline bool isEmergencyStop() {
      return isFailSave() || isLostFrame();
    }

    /** return failsafe from last update **/
    inline bool isFailSave() {
      return bbd->data.failsafe;
    }

    /** return lostframe from last update **/
    inline bool isLostFrame() {
      return bbd->data.lost_frame;
    }

    /** DO NOT USE THIS setting - maybe it's override your real arming state !!!!! Using only for tests **/
    inline void setArmed(bool v=true) {
      bbd->data.is_armed = v;
    }

    // is true, if arming is allowed and arm-switch is in ARM-Position
    inline bool isArmed() {
      Serial.print("isARMED("); Serial.print(bbd->data.is_armed); Serial.println(")");
      return bbd->data.is_armed ;
    }

    // return the current state of the arming switch (from real channel)
    inline bool isArmingChannelON() {
      return (bbd->data.ch[ARMING] > ARMING_VALUE)?true:false ;
    }

  private:
    // check if arming is allowed, if not arming is set to false
    bool ArmingAllowed();                                 // is used during Receiver::read(), this method set _isPreventArming to true/false

  private:
    HardwareSerial *_bus;                     // serial bus to receiver
    uint8_t _txpin, _rxpin;                   // serial pins
    uint8_t armingMask;                       // bit 7,6,5,4 = gimbal/poti, 3=unsed(allways 1), 2=GimbalInCP, 1=Arming
    uint8_t armingOKMask = 0b11111110;        // Arming is OFF
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
    uint16_t channel_calibration [NUMBER_CHANNELS][6] = {
      // calibrations values for 16 channels
      // idx0= SBUS lowes value, idx1=SBUS Center, idx2=SBUS highest value, idx3+4=calibrated min/max value
      {192, 0, 1800,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {192, 0, 1794,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {192, 0, 1794,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {192, 0, 1794,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX}, 
      {173, 0, 1810,  GIMBAL_MIN, GIMBAL_CENTER_POSITION, GIMBAL_MAX} 
    };

};


#endif