# PODRacer - Serial Reader

This python script is used to collect live data send by PODRacer FW.

The serial read recevie messages which are send via serial interface and store them into a csv file.

## Runtime Arguments
- --port : define your serial port. If locked, system ends
- --baud : default is 115200, it's possible to set a individual baud rate, must be the same as in fw.
- --tout : timeout, default is 5secs
- --opath : default is `data`, absolute path were file to store
- --delimter : default is ',' (comma), should be the same as in firmware
- --use_ts : if set, a current timestamp is set for filename (should be used !)
- --v : output verbosity

**Example**
```./serial_reader.py --port /dev/cu.usbserial-0001 ```


## CSV structure
```
TIME,TASK,GROUP,CH_R,CH_P,CH_Y,CH_H,CH_T,ARMING,AUX2,AUX3,float0,float1,float2,float3,float4,float5,float6,float7,ldata0,ldata1,ldata2,ldata3,ldata4,ldata5,ldata6,ldata7,pidRoll,pidPitch,pidYaw,pidThrust,pidHover,HOVER_MINIMAL_HEIGHT,HOVER_MIN_DISTANCE,HOVER_MAX_DISTANCE,CRC
```
- TIME : in milli seconds
- TASK : Task name
- GROUP : can be set by TASK - default should be 'UPD'. Is used to separate different output from a task
- CHR_R - CH-T : channel data which es send to receiver
- ARMING :  should allways 1
- AUX2+3 : data from AUX-_Channels
- float0-7 : depends on task, if a task contain several sensors (groups), can be splittet into several parts
- ldata0-7 : see float0-7 
- pidRPYTH : special pid values for channels
- HOVERxxx : constant values for hover task
- CRC : checksum
- 