/*




*/
#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <HardwareSerial.h>
#include "global.h"
#include "global_utils.h"
#include "TaskData.h"
#include <FastCRC.h>

class SLog {
  public:
    SLog(HardwareSerial *bus, int baud=115200, uint8_t level=3) {
      _bus = bus;
      _bus->begin(baud);
      _level = level;
      _visualizer_mode = 1;
    
    }

    /** set or reset the output format to visualizer-mode or human-readable-mode, based on parameter 'on'. Default is ON=1, OFF=0 **/
    void setVisualizerMode(bool on=1) {
      this->_visualizer_mode = on;
    }

    void setLoglevel(uint8_t level=3) {
      _level = level;
    }
    void error(const long v, const char *tname="?", bool cr=true) {
      sprintf(buffer, "%i", v);
      error(buffer, tname, cr);
    }

    void error(const char *text, const char *tname="?", bool cr=true) {
      if (_level >= 1) {
        _print(text, "ERROR", tname, cr);
      }
    }
    void warn(const long v, const char *tname="?", bool cr=true) {
      sprintf(buffer, "%i", v);
      warn(buffer, tname, cr);
    }

    void warn(const char *text, const char *tname="?", bool cr=true) {
      if (_level >= 2) {
        _print(text, "WARN", tname, cr);
      }
    }

    /** create a 16Bit CRC sum from buffer **/
    uint16_t getCRC(const char *buf) {
      FastCRC16 CRC16;
      return CRC16.ccitt((uint8_t*)buf, strlen(buf));
    }

    /** add a 16Bit CRC sum at the end of buffer **/
    void addCRC2Buffer(char *buf) {
        char crc_buf[15];
        sprintf(crc_buf,",%08X", getCRC(buf));
        // put crc_buf at the end of buf-address
        memcpy(buf+strlen(buf), crc_buf, sizeof(crc_buf));
    }

    /** extend 'fromBuf' with 'toBuf' **/
    void addBuffer(char *toBuf, const char *fromBuf) {
      memcpy(toBuf+strlen(toBuf), fromBuf, sizeof(fromBuf));
    }

    /************************************************************************/
    /* once_xxxx methods print the statement only one time and set value    */
    /* on address_value to 1. Up from this point, no additional loggings.   */
    /* are possible. Nice feature inside loops in tasks                     */
    /************************************************************************/

    void once_info(uint16_t *once_mask, uint8_t maskBit, const char *text, const char *tname="?", bool cr=true) {
      (maskBit > 15)?maskBit=15:maskBit;
      if (bitRead(*once_mask, maskBit) == 0) {
        info(text, true, tname, cr);
      }
      bitSet(*once_mask, maskBit);
    }

    void once_warn(uint16_t *once_mask, uint8_t maskBit, const char *text, const char *tname="?", bool cr=true) {
      (maskBit > 15)?maskBit=15:maskBit;
      if (bitRead(*once_mask, maskBit) == 0) {
        warn(text, tname, cr);
      }
      bitSet(*once_mask, maskBit);
      
    }

    void once_error(uint16_t *once_mask, uint8_t maskBit, const char *text, const char *tname="?", bool cr=true) {
      (maskBit > 15)?maskBit=15:maskBit;
      if (bitRead(*once_mask, maskBit) == 0) {
        error(text, tname, cr);
      }
      bitSet(*once_mask, maskBit);
    }

    void once_data(uint16_t *once_mask, uint8_t maskBit,TaskData *td, const bool allowLog, const char *tname="?", const char *tgroup="-", bool printCH=true, bool printFData=false, bool printLData = false, bool pidData = false) {
      (maskBit > 15)?maskBit=15:maskBit;
      if (bitRead(*once_mask, maskBit) == 0) {
        data(td, allowLog, tname, tgroup, printCH, printFData, printLData, pidData);
      }
      bitSet(*once_mask, maskBit);
    }

    void once_binary(uint16_t *once_mask, uint8_t maskBit,const char *text, const char* tname, const uint8_t v, bool cr=true){
      (maskBit > 15)?maskBit=15:maskBit;
      if (bitRead(*once_mask, maskBit) == 0) {
        printBinary(text, true, tname, v, cr);
      }
      bitSet(*once_mask, maskBit);

    }

    /************************************************************************/
    #
    /** is used to log data as output for visualizer or human-readable **/
    void data(TaskData *data, const bool allowLog, const char *tname="?", const char *tgroup="-", bool printCH=true, bool printFData=false, bool printLData = false, bool pidData = false) {
      char tg[20];
      sprintf(tg,"%s_%s", tname, tgroup);

      memset(buffer, '\0', sizeof(buffer));
      memset(tmp, '\0', sizeof(tmp));

      if (_visualizer_mode == 1 && allowLog) {
        sprintf(buffer, "\nFEEF,%d,%d,%d,%s,%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i",
        (long)data->data.start_millis,
        (long)data->data.end_millis,
        ((long)data->data.end_millis - (long)data->data.start_millis),
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
        data->data.const_hover[2],
        data->data.const_hover[3]);
        addCRC2Buffer(buffer);
        print(buffer, true);
      }
      else {
        // human-readable-output
        if (allowLog) {
          if (printCH) {
            sprintf(tmp, 
              "(%3s)RUN(%3d) UPD:%d,ARM:%d,FAIL:%d,LF:%d, CH(R,P,Y,H,T,ARM,AUX2-3): %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d",
              tgroup,
             ((long)data->data.end_millis - (long)data->data.start_millis),
              data->data.updated,
              data->data.is_armed,
              data->data.failsafe,
              data->data.lost_frame,
              data->data.ch[ROLL],
              data->data.ch[PITCH],
              data->data.ch[YAW],
              data->data.ch[HOVERING],
              data->data.ch[THRUST],
              data->data.ch[ARMING],
              data->data.ch[AUX2],
              data->data.ch[AUX3]
            );
            info(tmp, allowLog, tname, true);
          }
          if (printLData) {
            sprintf(tmp, 
              "LONG (0-7):%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
                data->data.ldata[0],
                data->data.ldata[1],
                data->data.ldata[2],
                data->data.ldata[3],
                data->data.ldata[4],
                data->data.ldata[5],
                data->data.ldata[6],
                data->data.ldata[7]
            );
            info(tmp, allowLog, tname, true);
          }
          if (printFData) {
            sprintf(tmp, 
              "FLOAT(0-7):%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
              data->data.fdata[0],
              data->data.fdata[1],
              data->data.fdata[2],
              data->data.fdata[3],
              data->data.fdata[4],
              data->data.fdata[5],
              data->data.fdata[6],
              data->data.fdata[7]
            );
            info(tmp, allowLog, tname, true);
          }
          if (pidData) {
            sprintf(tmp, 
              "PID  (RPYTH):%d\t%d\t%d\t%d\t%d",
              data->data.pid_rpyth[0],
              data->data.pid_rpyth[1],
              data->data.pid_rpyth[2],
              data->data.pid_rpyth[3],
              data->data.pid_rpyth[4]
            );
            info(tmp, allowLog, tname, true);
          }
        } // allowLog
      }
    }

    void info(const long v, bool allowLog, const char *tname="?", bool cr=true) {
      sprintf(buffer, "%i", v);
      info(buffer, allowLog, tname, cr);
    }

    void info(const double v, bool allowLog, const char *tname="?", bool cr=true) {
      sprintf(buffer, "%f", v);
      info(buffer, allowLog, tname, cr);
    }
    
    void info(const char *text, bool allowLog, const char *tname="?", bool cr=true) {
      if (_level >= 3 && allowLog) {
        _print(text, "INFO", tname, cr);
      }
    }

    void print(const long v, bool cr=true) {
      sprintf(buffer, "%i", v);
      print(buffer, cr);
    }

    void print(const char *text, bool cr=false) {
      if (cr) {
        _bus->println(text);
      }
      else {
        _bus->print(text);
      }
    }

    void printBinary(const char *text, bool allowLog, const char* tname, const uint8_t v, bool cr=true) {
      info(text, allowLog, tname, false);
      sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
      if (cr) {
        _bus->println(buffer);
      }
      else {
        _bus->print(buffer);
      }
    }

    /** convert v into a bit buffer array, pattern=0|1 default = 1 = '0000 0000' **/
    void getBinary(char* buffer, const uint8_t v, uint8_t pattern=1) {
      switch(pattern) {
        case 0:
          sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
          break;
        case 1:
          sprintf(buffer, BYTE_TO_BINARY_PATTERN2, BYTE_TO_BINARY(v));
          break;
        default:
          sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(v));
      }
      // return buffer;
    }

    void convertValueToBinary(char* buffer, size_t const size, void const * const ptr) {
      unsigned char *b;
      unsigned char c;
      uint8_t idx;
      char x[64];

      for (size_t i = size; i > 0; i--) {
        b = ((unsigned char* )ptr+i);
        Serial.println(*b);
        for (int8_t j=7; j >= 0; j--) {
          c = (*b & 1);
          sprintf(x,"b(%d) i(%d) j(%d) idx(%d) c(%d)|", *b, i, j, idx, c);
          _bus->println(x);          
        }
      }

      // for (int8_t i = size-1; i >= 0; i--) {
      //   for (int8_t j=7; j >= 0; j--) {
      //     idx = (i*7) + j + 1;
      //     *b = ((unsigned char* )ptr)[i];
      //     c = ((b[i] >> j) & 1 > 0)?'1':'0';
      //     c = (b[i] >> j) & 1;
      //     //buffer[idx] = c;
      //     sprintf(x,"b(%d) i(%d) j(%d) idx(%d) c(%d)|", *b, i, j, idx, c);
      //     _bus->println(x);
      //   }
      //}
      //_bus->println(buffer);
    }

    void debug(const long v, bool allowLog, const char *tname="?", bool cr=true) {
      sprintf(buffer, "%d", v);
      debug(buffer, allowLog, tname, cr);
    }

    void debug(const char *text, bool allowLog, const char *tname="?", bool cr=true) {
      if (_level >= 4 && allowLog) {
        _print(text, "DEBUG",tname, cr);
      } 
    }

  private:
    HardwareSerial *_bus;
    uint8_t _level;
    char buffer [300];
    bool _visualizer_mode ;
    char tmp[300];

  private:
    void _print(const char *text, const char *LEVEL, const char *tname="?", bool cr=true) {
      memset(buffer, '\0', sizeof(buffer));
      sprintf(buffer,"%5s|%5s|%s", LEVEL, tname, text);
      if (cr) {
        _bus->println(buffer);
      }
      else {
        _bus->print(buffer);
      }
      _bus->flush();
    }
};



#endif
