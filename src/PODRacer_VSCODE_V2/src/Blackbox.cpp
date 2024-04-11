
#include "Blackbox.h"

Blackbox::Blackbox(SLog *log, uint8_t cs) {
  _cs = cs;
  logger = log;
  blackboxAvailable = false;
  fOpen = false;
  _tname = "BBOX";

  sprintf(fw, FW_VERSION_PATTERN,
    FW_VERSION_MAJOR,
    FW_VERSION_MINOR,
    FW_VERSION_PATCH
  );

}

void Blackbox::clearStruct(TaskData *bbd, uint8_t task_id, uint8_t groupA, uint8_t groupB) {
  memset (bbd, 0, sizeof(TaskData));
  bbd->data.task_id = task_id;
  bbd->data.groupA = groupA;
  bbd->data.groupB = groupB;
}

bool Blackbox::begin(void) {
  logger->info("Blackbox initializing ...", true, _tname);
  blackboxAvailable = false;
  fOpen = false;
  if (!SD.begin(_cs)) {
    // return true, because if no SD card attached, we can't store 
    // blackbox data, but we can fly :-)


    logger->error("Blackbox::Card mount failed", _tname);
    return true;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    logger->error("Blackbox::No SDCard attached", _tname);
    return true;
  }
  blackboxAvailable = true;

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  sprintf(buffer, "Blackbox::SD Card Size: %lluMB", cardSize );
  logger->info(buffer, true, _tname);

  File f = SD.open(cfgFile);
  bool ok = false;
  uint8_t cnt = 0;

  // check if config file available, if not create it and wirte 0001
  if (!f) {
    if (writeFile(SD, cfgFile, "0") > 0) {
      logger->error("Blackbox::Failed to write config file", _tname);
      return true;
    };
  }

  uint8_t rc;
  char _buf[5];
  for (uint8_t i=5;i--;) {_buf[i]='\0';}
  rc = readBytes(SD, cfgFile, _buf, 4);
  sprintf(buffer, "Blackbox::(%d) Read Buffer: %d", rc, sizeof(_buf));
  logger->debug(buffer, _tname);
  // convert index and create next
  int idx = 0;
  sscanf(_buf, "%d", &idx);
  sprintf(buffer, "/blackbox_%04d.bbd", idx);
  logger->info(buffer, true, _tname);

  bbf = open(SD, buffer, FILE_APPEND, true);
  if (bbf) {
    fOpen=true;
    blackboxAvailable = true;
    idx++;
    sprintf(_buf, "%d", idx);
    if (writeFile(SD, cfgFile, _buf) > 0) {
     logger->error("Blackbox::Failed write config file", _tname);
    }
    sprintf(buffer, "Blackbox::Next file index: %d", idx);  }
  else {
    blackboxAvailable = fOpen=false;
    //Serial.println("Failed to create new blackbox file");
  }
  return blackboxAvailable;  
}

void Blackbox::update(void) {

}

void setFWVersion(TaskData *data) {
}

void Blackbox::update(TaskData *data){
  if (bbf) {
    memcpy(data->data.fwversion, fw, sizeof(data->data.fwversion));
    bbf.write(data->bytes, sizeof(TaskDataStruct));
    bbf.flush();
  }
}


//------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------

fs::File Blackbox::open(fs::FS &fs, const char * path, const char* mode, const bool create){
  return fs.open(path, mode, create);
}

uint8_t Blackbox::readBytes(fs::File &f, char *buf, size_t length){
  uint8_t rc=0;
  if (!f) {
    rc=1;
  }
  else {
    while(f.available()) {
      f.readBytes(buf, length);
    }
    rc=0;
  }
  return rc;
}

uint8_t Blackbox::readBytes(fs::FS &fs, const char *path, char *buf, size_t length) {
  uint8_t rc = 0;
  File f = fs.open(path);
  if (!f) {
    rc=1;
  }
  else {
    size_t len = f.size();
    if (length > len) {
      length = len;
    }
    //Serial.print("read ("); Serial.print(length); Serial.print(") BufferSize: ");Serial.print(sizeof(buf));
    //Serial.println();
    f.readBytes(buf, length);
    rc=0;
  }
  f.close();
  return rc;
}

/** open - write - close **/
uint8_t Blackbox::writeFile(fs::FS &fs, const char * path, const char * message){
  uint8_t rc = 0;
  File f = fs.open(path, FILE_WRITE);
  if (!f) {
    rc=1;
  }
  else if (f.print(message)) {
    rc=0;
  }
  else {
    rc=2;
  }
  f.close();
  return rc;
}

/** write/append **/
uint8_t Blackbox::writeFile(fs::File &f, const char * message){

}

uint8_t Blackbox::appendFile(fs::File &f, const char * message){
  if (f) {
    //Serial.println("<a>");
    if (f.print(message)) {
      return 0;
    }
    return 2;
  }
  return 1;
}

uint8_t Blackbox::appendFile(const char * message){
  return appendFile(bbf, message);
}
