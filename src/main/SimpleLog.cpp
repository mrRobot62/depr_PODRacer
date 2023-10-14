
#ifndef _SIMPLELOG_
#define _SIMPLELOG_
#include <HardwareSerial.h>

//namespace podr {
  class SLog {
    public:
      SLog(HardwareSerial *bus, int baud=115200, uint8_t level=3) {
        _bus = bus;
        _bus->begin(baud);
        _level = level;
      }

      void setLoglevel(uint8_t level=3) {
        _level = level;
      }
      void error(const long v, bool cr=true) {
        sprintf(buffer, "%d", v);
        error(buffer, cr);
      }
      void error(const char *text, bool cr=true) {
        if (_level >= 1) {
          _print(text, "ERROR", cr);
        }
      }
      void warn(const long v, bool cr=true) {
        sprintf(buffer, "%d", v);
        warn(buffer, cr);
      }
      void warn(const char *text, bool cr=true) {
        if (_level >= 2) {
          _print(text, "WARN", cr);
        }
      }

      void simulate(const char *text) {
        _print(text, "SIMUALTE", true, true);
      }

      void info(const long v, bool cr=true) {
        sprintf(buffer, "%d", v);
        info(buffer, cr);
      }

      void info(const char *text, bool cr=true) {
        if (_level >= 3) {
          _print(text, "INFO", cr);
        }
      }

      void print(const long v, bool cr=true) {
        sprintf(buffer, "%d", v);
        print(buffer, cr);
      }

      void print(const char *text, bool cr=false) {
        if (cr) {
          _bus->print(text);
        }
        else {
          _bus->println(text);
        }
      }

      void debug(const long v, bool cr=true) {
        sprintf(buffer, "%d", v);
        debug(buffer, cr);
      }
      void debug(const char *text, bool cr=true) {
        if (_level >= 4) {
          _print(text, "DEBUG", cr);
        }
      }

    private:
      HardwareSerial *_bus;
      uint8_t _level;
      char buffer [100];
    private:
      void _print(const char *text, const char *LEVEL, bool cr=true, bool simulate=false) {
        if (simulate) {
          _bus->println(text);
        }
        else {
          sprintf(buffer,"%10d|%5s|%s", millis(), LEVEL, text);
          if (cr) {
            _bus->println(buffer);
          }
          else {
            _bus->print(buffer);
        }
        }
        _bus->flush();
        /*
        _bus->print(millis()); 
        _bus->print('|') ; _bus->print(LEVEL); _bus->print('|');
        if (cr) {
          _bus->println(text);
        }
        else {
          _bus->print(text);
        }
        */
      }
  };

//};


#endif