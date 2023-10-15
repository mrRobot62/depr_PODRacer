# POD-Racer



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

## Debugging & Logging
SimpleLog include some output function to adjust log output. To avoid thousands of log messages, inside constants.h there are some `#define` statements to adjust the output

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