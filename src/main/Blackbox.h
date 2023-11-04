#ifndef _BLACKBOX_H_
#define _BLACKBOX_H_


#include "Arduino.h"
//#include "Task.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "SimpleLog.cpp"
#include <sbus.h>
#include "TaskData.h"

//class Blackbox : public TaskAbstract {
class Blackbox {
  public:
    /** Constructor taskID, logger, cs_pin **/
    //Blackbox(uint8_t taskID, SLog *log, uint8_t cs);
    Blackbox(SLog *log, uint8_t cs);

    bool begin(void);
    void update(void);
    void update(BBD *data);

    void close() {
      if (bbf) {
        bbf.close();
      }
    }

    /** reset BBD struct, can set task_id, groupA, groupB if set**/
    void clearStruct(BBD *bbd, uint8_t task_id=0, uint8_t groupA=0, uint8_t groupB=0);

    char *FWVersioin(void) {
      return fw;
    }

  private:
    SLog *logger; 
    uint8_t _cs;
    bool blackboxAvailable;
    bool fOpen;
    fs::File bbf;
    char *_tname;
    char buffer[50];
    char cfgFile[14] = "/blackbox.txt";  // please note \0 is added to the string
    char fw[10] = {0};

    /** return file handle for "path-file". Default as READ-ONLY **/
    fs::File open(fs::FS &fs, const char *path, const char* mode = FILE_READ, const bool create = false);

    /** read data return data in buffer with size_t bytes **/        
    uint8_t readBytes(fs::File &f, char *buffer, size_t length);

    /** open - read - close return data in buffer with size_t bytes **/
    uint8_t readBytes(fs::FS &fs, const char *path, char *buf, size_t length);

    /** open - write - close write message into file  **/
    uint8_t writeFile(fs::FS &fs, const char *path, const char * message);

    /** append/(over)write into open &f.  Write message  **/
    uint8_t writeFile(fs::File &f, const char * message);
    uint8_t appendFile(fs::File &f, const char * message);
    uint8_t appendFile(const char * message);



};

#endif