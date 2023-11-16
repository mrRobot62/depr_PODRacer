# POD-Racer
```
.---------------------------------------------------------.
|_______________________________                          |
|___  __ \_  __ \__  __ \__  __ \_____ ___________________|
|__  /_/ /  / / /_  / / /_  /_/ /  __ `/  ___/  _ \_  ___/|
|_  ____// /_/ /_  /_/ /_  _, _// /_/ // /__ /  __/  /    |
|/_/     \____/ /_____/ /_/ |_| \__,_/ \___/ \___//_/     |
'---------------------------------------------------------'
```

# Hardware
## Transmitter (Taranis/TBS)
Channel configuration
- CH0 : roll axis		-> gimbal roll
- CH1 : pitch axis		-> gimbal pitch
- CH3 : FMS Impeller		-> gimbal throttle
- CH4 : yaw axis			-> gimbal yaw
- CH5 : arming/disarming-> switch
- CH6 : -> switch
- CH7 : -> switch
- CH8 : Hovering via POT S1	
## ToF-Sensors
- PMW3901 
- VL53LX1
- TF Mini LIDAR

# Software
# Arduino IDE 2.2.1

## ESP 32 - Datatype size
|Type|DataType|size (bytes)|
|---|---|:-:|
|Bool|bool|1|
|Char|char/int8-t|1|
|signed byte|int8_t|1|
|unsigned byte|uint8_t|1|
|int|int16_t|2|
|uint|uint16_t|2|
|long|long/int32_t|4|
|unsigned long|uint32_t|4|
|long64|int64_t|8|
|unsigned long64|uint64_t|8|
|Float|float|4/(8)|
|Double|double|8|
|String|char */String|variable|
|Bytes|uint8_t|variable|
||||

## Blackbox 
To store flight data, we use a SDCard with 4-8GB size. Every task use an internal data structure (every task use the same structure). This structure is used to store the data in a blackbox_<seqnumber>.bbd.

If PODRacer is armed a new blackbox file will be created, if disarmed the file is stored and closed on disk.
If disarming was not available (e.g. power off) the file is corrupted and not usable.

### Datetype-Struct (for every tasks usable)
```
#define NUMBER_CHANNELS 16
#define DATA_SIZE 8
typedef struct {
  uint16_t header;				   // fix to 0xFEEF
  long millis;                     // milliseconds
  // data
  uint8_t task_id;                 // produces by task-id
  uint8_t updated;                 // set by tasks
  uint8_t failsafe;                // set by receiver
  uint8_t lost_frame;              // set by receiver
  uint8_t levelA;                  // 1=normal, 2=depth (1=default)
  uint8_t levelB;                  // group_id 0=misc, 1=hover, 2=surfacedist, 3=OpticalFlow, 4=mixer, 
  uint16_t ch[NUMBER_CHANNELS];    // channel data (0=ch1, 1=ch2, 2=ch3, ...)
  //uint16_t ch_w[NUMBER_CHANNELS];// channel data (0=ch1, 1=ch2, 2=ch3, ...) WRITE 
  long ldata[DATA_SIZE];           // an be used for long values  
  double fdata[DATA_SIZE];         // can be used for float values
  double pid_rpyth[DATA_SIZE];     //(R/P/Y/T/H);
  uint16_t lock;                   // fix to 0xFF
  uint16_t crc;
} BlackBoxStruct;

typedef union {
  BlackBoxStruct data;
  uint8_t bytes [sizeof(BlackBoxStruct)];
} BBD;
```

Dump from a simulated data structure with fixed values
```
  bbd->data.header = 0xFEEF;
  bbd->data.millis = millis();
  bbd->data.task_id = 0x0A;
  bbd->data.updated = 0x0B;
  bbd->data.failsafe = 0xF0;
  bbd->data.lost_frame = 0xC0;
  bbd->data.groupA=0xB0;
  bbd->data.groupB=0xA0;
  bbd->data.crc = 0;  // for initializing!!!!! 
  for (uint8_t i=NUMBER_CHANNELS; i--;) {
    bbd->data.ch[i] = 1450;
  }
  for (uint8_t i=DATA_SIZE; i--;) {
    bbd->data.ldata[i] = -500;
    bbd->data.fdata[i] = 3.1415;
    bbd->data.pid_rpyth[i] = 1.0;
  }
  bbd->data.lock = 0xFF;
  uint16_t crc = FCRC16.ccitt((uint8_t*)bbd->bytes, sizeof(bbd));
  bbd->data.crc = crc;  
  sprintf(buffer, "CCITT-CRC: 0x%X", crc);
  Serial.println(buffer);
```
To understand how the struct is aligned you should understand [Data structure alignment](https://en.wikipedia.org/wiki/Data_structure_alignment)

### Short description
Bytes are stored with little endian. (LSB first, than MSB)

|Position| Data| Type |Padding| Info|
|---|:-:|:-:|:-:|---|
|0-1|EF FE|uint16_t|yes|header 2 bytes with padding|
|2-3|00 00|-|padding|2 padding bytes for uint16_t|
|4-7|1D 02 00 00|long|no|millis()|
|8-13|0A 0B F0 C0 B0 A0|uint8_t|no|7 Bytes uint8_t (task_id...groupB)|
|14-45|AA 05....|uint16_t|yes|16 channels a 2 Bytess, set to 1450(dec) 0x05AA|
|46,47|00 00|-|padding|2 padding bytes for uint16_t||
|48-79|0C FE FF FF|long|no|4 Bytes LData. Note: array of 8 long values|
|80-143|6F 12 83 C0 CA 21 09 40|double|no|8 Bytes FData. Note: array of 8 doubles values|
|144-207|00 00 00 00 00 00 F0 3F|double|no|8Bytes per pid_rpyth. Note: array of 8 doubles |
|208|FF|uint8_t|yes|lock byte|
|209|00|no|padding|1 padding byte|
|210-211|97 2C|uint16_t|yes|2 Bytes CRC|
|212-215|00 00 00 00 |-|padding| 4 Bytes padding|

```
-- dump-start (216 bytes) -------------------------
EF FE 00 00 1D 02 00 00 0A 0B F0 C0 B0 A0 AA 05 
AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 
AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 00 00 
0C FE FF FF 0C FE FF FF 0C FE FF FF 0C FE FF FF 
0C FE FF FF 0C FE FF FF 0C FE FF FF 0C FE FF FF 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
FF 00 97 2C 00 00 00 00 
-- dump-end ------------------------------------
```

## Debugging & Logging
SimpleLog include some output function to adjust log output. To avoid thousands of log messages, inside constants.h there are some `#define` statements to adjust the output. **Logging information will not be stored on SDCard !**

```
#define LOGLEVEL 3  // 3=Info, 4=Debug

//#define LOG_TASK_ALL
//#define LOG_TASK_RECEIVER
//#define LOG_TASK_ARBITRATE
#define LOG_TASK_OPTICALFLOW
//#define LOG_TASK_SURFACE1
//#define LOG_TASK_SURFACE2
#define USE_SERIAL_PLOTTER

``` 
- loglevel 3 (Info), 4=Debug messages
- **LOG_TASK_ALL:** if uncommented everything is logged
- **LOG_TASK_RECEIVER:** log only receiver output messages
- **LOG_TASK_ARBITRATE:** log only messages from Mixer
- **LOG_TASK_OPTICALFLOW:** log only messages from OpticalFlow class (PMW3901)
- **LOG_TASK_SURFACE1:** log only messages from TFMiniLidar
- **LOG_TASK_SURFACE2:** log only messages from VH53L1 ToF
- **USE_SERIAL_PLOTTER:** in combination of above directives you can produce nice SerialPlotter graphs


# Arduino Modules
- Bitcraze PMW391 v1.2 (ToF Sensor)
- Servo by Michael Margolis, v1.2.1
- PID by Brett Beauregard v1.2.0
- SimpleKalmanFilter by Denys Sene v0.1
- VL53LX1 by Pololu v1.3.1 (ToF Sensor)
- TFMPlus by Bud v1.5.0 (TFMini Lidar)
- SBUS Bolder FlightSystem by Brian Taylor v8.1.4 (Receiver) 