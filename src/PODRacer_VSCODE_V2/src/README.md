# PODRacer
A FPV 4 Motor copter construction with a fifth motor as thruster controlled by a flight controller with a BetaFlight firmware and a second PODRacer-FlightController based on an ESP32 with a specified firmware from this repo.

## Short summary of the base functionalities
### Betaflight-Flightcontroller
This FC is responsible for hovering, arming, disarming other saftey features. In Betaflight a preset should be used for heavy 5" copters to get a better performance for filters, PIDs ...
The base flight configuration is with ANGLE-mode enabled. In this case we will have more or less a self-stabilized copter

### general Channel-Mapping
* Channel 1 = ROLL
* Channel 2 = PITCH
* Channel 3 = THRUSTER (Throttle-Gimbal). This will spin impeller (5th) motor
* Channel 4 = Yaw
* Channel 5 = ARMING/DISARMING
* Channel 6 = HOVERING (POTI), this will spin motors 1-4 faster or slower
* Channel 7 = free
* Channel 8 = free
* 
### PODRacer-Flightcontroller
This FC handles all PODRacers specials. The FC controls several sensors and is responsible for hovering-height.

#### Tasks & Sensors
- Receiver-Task: This task will read an SBUS input signal, send this data (up to 16 Channels) to the firmware. Can write an SBUS-Signal to the BF-FC. The receiver is "man in the middle" between receiver and BF-FC
- Mixer-Task: this task collect all inputs form other tasks, mix them up, generate an output-data struture and send it back to the receiver (write-signal). The mixer implements a sub-sumption algorithm that will prioritize the tasks. Higher prioritised tasks will overwrite lower tasks. Example hovering is the lowes priority, Saftey-Task (not realy a task) but this have the hightest priority. SurfaceDistance sensor data are the hightest prioritized sensor data
- Hovering-Task: This is not a sensor. The PODRacer should hover in a height between 300-500mm over ground. With the POTI on the transmitter we start hovering. The sensors below with analyse the current heigt and correct the hovering signal send to the BF-FC
- Surface-Distance-Sensor: these are two sensors. A TFMini-LIDAR sensor at front bottom of the PODRacer and a second TOF-Sensor at bootom rear of the copter. The firmware anlayse the sensors inputs, calculate difference between target height and current height. The result is an input for the internal mixer task
- OpticalFlow-Sensor: this sensor is used to analyse if the PODRacer glides over ground during slow movements or if stops over ground. If an movement is recognized, the task calculate an opposite direction mix for roll, yaw, pitch axis and this information are input data for the mixer
- Steering-Task: This task do only some small adjustments if the PODRacer move in an arc and adjust roll/yaw/pid to get a smoother turn-alignment
  
#### Saftey
The arming-signal for the BF-FC is replaced by an PREVENT-Arming algorithm from the PODRacers FW. Arming is only allowed, if thruster is on min postion, hovering is on min postion and all other gimbals are in center position. If this is true, than arming is allowed. If armed, the for motors will spin and with the POTO the hovering (starting) is possible

## Main-Flightcontroller
any F405 or F722 controller, with latest Betaflight FW flashed

## PODRacers-Flightcontroller
an ESP32 WROOM (should running on other ESP32, but only tested with a WROOM)
flashed with latest PODRacer FW


# Transmitter configuration
We tested the system with transivers based on OpenTX. Below description describe only what you have to do on an OpenTX FW (or equivalent Systems like EdgeTX).
Very important: current FW for PODRacer do only work with SBUS-protocol - no CRSF, nothing other !!! Please double check if your receiver is configured as an SBUS-Receiver

## Configure Input/Output channels


# Betaflight - configuration

#√# Configure Channels

# Thurster-ESC connected as an servo output
The ESC for the thruster-brushless motor is an single esc connected with the PWM-input pin on the LED_OUTPUT pin from the main flight controller.

# PODRacer FlightController
## Flashen

