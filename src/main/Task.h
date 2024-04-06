#ifndef _TASK_H_
#define _TASK_H_

#include "TaskData.h"
#include "SimpleLog.cpp"
#include "constants.h"
#include "Blackbox.h"
#include <SimpleKalmanFilter.h>
#include <CoopSemaphore.h>
#include <sbus.h>
#include <FastCRC.h>

//namespace podr {



  class TaskAbstract {
    public:
      TaskAbstract(uint8_t taskID, SLog *log, Blackbox *bb=nullptr, HardwareSerial *visBus=nullptr) {
        logger = log;
        _id = taskID;
        _blackbox = bb;
        _tname = "?";
        _tgroup= "";
        _visBus=visBus;
      };
      virtual bool begin(void) = 0;
      virtual void update(void) = 0;
      virtual void taskName(char *tname) {
        _tname = tname;
      };

      inline const char* tName(void) {
        return _tname;
      }

      inline const bool hasError(void) {
        return (errorCode > 0)?true:false;
      }

      inline const uint8_t getError(void) {
        return errorCode;
      }

      inline void resetError() {
        errorCode = 0;
      }

      /** get data struct **/
      inline BBD data() const {return _bbd;}
      /** set data struct **/
      inline BBD data(BBD data) { _bbd = data;}

      inline uint8_t getID() {return _id;};

      inline bool isUpdated() {
        return _bbd.data.updated;
      }

      inline void setUpdateFlag() {
        _bbd.data.updated=true;
      }

      inline void setMilliseconds() {
        _bbd.data.millis = millis();
      }

      inline void resetUpdateFlag() {
        _bbd.data.updated=false;
      }
      
      /** can be used to reset the current task data structure. Attribute task_id is set to current task_id **/
      inline void clearStruct() {
        memset (&_bbd, 0, sizeof(_bbd));
        _bbd.data.task_id = _id;     
      }

      /** create a 16Bit CRC sum from buffer **/
      uint16_t getCRC(const char *buf) {
        return CRC16.ccitt((uint8_t*)buf, strlen(buf));
      }

      /** add a 16Bit CRC sum at the end of buffer **/
      void addCRC2Buffer(char *buf) {
          char crc_buf[15];
          sprintf(crc_buf,",%08X", getCRC(buf));
          memcpy(buf+strlen(buf), crc_buf, sizeof(crc_buf));
      }

      /** prepare data to a visualizer byte stream and send it via serial to host **/
      void send2Visualizer(const char *tname, const char *tgroup, BBD *data) {
        char tg[20];
        sprintf(tg,"%s_%s", tname, tgroup);
        sprintf(buffer, "\nFEEF,%d,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i",
          (long)data->data.millis,
          tname,
          tgroup,
          tg,
          // channel data, can be absolut or relative, depends on task / group
          data->data.ch[ROLL],
          data->data.ch[PITCH],
          data->data.ch[YAW],
          data->data.ch[HOVERING],
          data->data.ch[THRUST],
          data->data.ch[AUX2],
          data->data.ch[AUX3],
          data->data.ch[ARMING],

          // typcalliy used for output from filters and other double values
          data->data.fdata[0],
          data->data.fdata[1],
          data->data.fdata[2],
          data->data.fdata[3],
          data->data.fdata[4],
          data->data.fdata[5],
          data->data.fdata[6],
          data->data.fdata[7],

          // long values typical for data to channels increas/decrease delta values
          data->data.ldata[0],
          data->data.ldata[1],
          data->data.ldata[2],
          data->data.ldata[3],
          data->data.ldata[4],
          data->data.ldata[5],
          data->data.ldata[6],
          data->data.ldata[7],

          // PID channels
          data->data.pid_rpyth[0],
          data->data.pid_rpyth[1],
          data->data.pid_rpyth[2],
          data->data.pid_rpyth[3],
          data->data.pid_rpyth[4],

          // constants (needed for visualizer)
          data->data.const_hover[0],
          data->data.const_hover[1],
          data->data.const_hover[2]
        );
        uint16_t crc = getCRC(buffer);
        //Serial.print(sizeof(buffer));
        if (serPortLocked == false) {
          serPortLocked = true;
          // concat a crc sum at the end of the buffer
//          char crc_buf[5];
//          sprintf(crc_buf,",%04X", crc);
//          memcpy(buffer+strlen(buffer), crc_buf, sizeof(crc_buf));
          //
          addCRC2Buffer(buffer);

          if (_visBus == nullptr) {
            //Serial.print("***");
            Serial.print(buffer);
          } 
          else {
            //Serial.print("+++");
            _visBus->print(buffer);
          }

          delay(5);
          serPortLocked = false;
          delay(5);
        }      
      };

      void send2VisualizerSBUS(const char *tname, const char *tgroup, bfs::SbusData *sbus) {
        char tg[20];
        sprintf(tg,"%s_%s", tname, tgroup);
        sprintf(buffer, "\nFEEF,%d,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i",
          (long)millis(),
          tname,
          tgroup,
          tg,
          // channel data, can be absolut or relative, depends on task / group
          sbus->ch[ROLL],
          sbus->ch[PITCH],
          sbus->ch[YAW],
          sbus->ch[HOVERING],
          sbus->ch[THRUST],
          sbus->ch[AUX2],
          sbus->ch[AUX3],
          sbus->ch[ARMING],
          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
          0, 0, 0, 0, 0, 0, 0, 0,
          0.0, 0.0, 0.0, 0.0, 0.0,
          0,0,0
        );
        uint16_t crc = getCRC(buffer);
        if (serPortLocked == false) {
          serPortLocked = true;
//          char crc_buf[5];
//          sprintf(crc_buf,",%04X", crc);
//          memcpy(buffer+strlen(buffer), crc_buf, sizeof(crc_buf));
          //
          addCRC2Buffer(buffer);
          //
          if (_visBus == nullptr) {
            //Serial.print("***");
            Serial.print(buffer);
          } 
          else {
            //Serial.print("+++");
            _visBus->print(buffer);
          }
          serPortLocked = false;
          delay(5);
        }
      };


    protected:
      char *_tname, *_tgroup;

      /** an error contain two parts. B7-B4 = TaskID, B3-B0 error code **/
      inline void setError(uint8_t taskID, uint8_t code) {
        taskID = constrain(taskID,1,15);      // no TaskID 0
        code = constrain(code,1,15);          // no ErrorCode 0
        errorCode = (taskID << 4) | code;
        resetUpdateFlag();
      }

      /** check if value is in a center postioin range, if not return value, if yes return centerValue **/
      inline int16_t centeredValue(int16_t value, int16_t centerValue, int8_t offset) {
        if ((value >= (centerValue - offset)) && (value <= (centerValue+offset))) {
          return centerValue;
        }
        return value;
      }

      /** check if value is >= min and value <= max **/
      inline bool isInRange(double value, double min, double max) {
        if ((value >= min) && (value <= max)) {
          return true;
        }
        return false;
      }


      /** check if value is >= value-offset and value <= value+offset **/
      inline bool isInInterval(int16_t value, int16_t offset) {
        if ((value >= (value-offset)) && (value <= (value+offset))) {
          return true;
        }
        return false;
      }

      /** check if value >= compareValue-offset and value <= compareValue+offset **/
      inline bool isInInterval(int16_t value, int16_t compareValue, int16_t offset) {
        if ((value >= (compareValue-offset)) && (value <= (compareValue+offset))) {
          return true;
        }
        return false;
      }


      /** check if a value (value) is in a range between compareValue-min and compareValue+max to due to some small noise from receiver values **/
      inline bool isInInterval(int16_t value, int16_t compareValue, int16_t min, int16_t max) {
        if ((value >= (compareValue+min)) && (value <= (compareValue+max))) {
          return true;
        }
        return false;
      }
      BBD _bbd;                     // Blackbox Data Struct
      Blackbox *_blackbox;
      SLog *logger;   
      CoopSemaphore *_sema;

      //
      // PID-Controller default values
      //
      // please check .h file from sensors



      //----- PID Controller for TFMiniLidar sensor (distance 1 - front)
      float kpTFMini = 0.25;
      float kiTFMini = 0.0;
      float kdTFMini = 0.25;

      //----- PID Controller for VH53L1 sensor ToF sensor (distance 2 - rear)
      //
      float kpVH53 = 0.25;
      float kiVH53 = 0.0;
      float kdVH53 = 0.25;

      //-----
      char buffer[300];
      uint8_t _blink_pattern;
      uint8_t _id;


      //------
      FastCRC16 CRC16;

    private:
      uint8_t errorCode;
      bool serPortLocked;
      HardwareSerial *_visBus;
  };
//};

#endif