#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import time
from datetime import datetime
import serial
import argparse
import sys
import os

# SYNC allways FEEF, if not ignore record
columns = [
            # grouping information
            "TIME", "TASK","GROUP",
            # channels 
           "CH_R", "CH_P", "CH_Y", "CH_H", "CH_T",
           "ARMING","AUX2","AUX3",
           # floats & long values, used by tasks
           "float0","float1", "float2", "float3", "float4", "float5", "float6", "float7",
           "ldata0","ldata1","ldata2","ldata3","ldata4","ldata5","ldata6","ldata7",
           # pid values used by tasks
           "pidRoll","pidPitch","pidYaw","pidThrust","pidHover",
           # constants.....
           "HOVER_MINIMAL_HEIGHT", "HOVER_MIN_DISTANCE", "HOVER_MAX_DISTANCE" 
           ];

fname = "podrdata{TS}.csv"

parser = argparse.ArgumentParser()

parser.add_argument("--port", type=str, help="insert your serial port")
parser.add_argument("--baud", type=int, default=115200, help="Baud rate, default=115200")
parser.add_argument("--tout", type=int, default=5, help="Timout in seconds, default=5")
parser.add_argument("--opath", type=str, default="./data", help="output path, default is ./data")
parser.add_argument("--delimiter", type=str, default=",", help="delimiter, default is comma (,)")
parser.add_argument("--use_ts", action="store_true", help="if set, use a timestamp in file name")
parser.add_argument("-v", "--verbose", action="count", default=1, help="verbosity output -v -vv -vvv")

args = parser.parse_args()


def createSerialObj(port, baud, tout):
    """
    create a pySerial object on a given port with baud and timeout

    Args:
        port (_type_): path to serial port
        baud (_type_): baud rate
        tout (_type_): timeout

    Return:
        serial obj
    """
    try:
        s = serial.Serial(port=port,baudrate=baud, timeout=tout)
        s.isOpen()
        print("SerialPort open");
    except IOError as err:
        print ("Port already in use. Try to close ...")
        s.close()
        print ("Port closed, try to reopen...")
        s.open()
    except Exception as err:
        print (err)
        print (f"problem to create serial object on '{port}' with '{baud}' baudrate")
        return None
    return s

def readSerialData(ser:serial, writer : csv.writer):
    """
    read data from serial port into data structure and return this struct

    Args:
        ser (serial): serial object
        data (dict): data structure

    Returns:
        dict: data structuse
    """
    ser.flushInput()
    raw = []
    timeout = False
    lines = lb = 0
    print("-----------------------------")
    print("waiting for serial data......")
    print("-----------------------------")
    while not timeout:
        line = ser.readline()
        l = line.decode('utf-8').splitlines()
        if (len(l) > 0):
            raw = l[0].split(',')
            if raw[0] != "FEEF":
                continue
        else:
            continue
        writer.writerow(raw[1:])
        if (lines % 100 == 0):
            if (lb % 10 == 0) and (lb > 0): 
                print("")
                lb = 0
            print(f"{lines:>6} ",end="")
            lb += 1
        lines += 1

def getFileName(path="./"):
    dt = datetime.fromtimestamp(time.time())
    ts = ""
    if args.use_ts:
        ts = dt.strftime("_%Y%m%d_%H%M%S")
    f = os.path.join(path, fname.format(TS=ts))
    return f

def createNewFile():
    fn = getFileName(args.opath)
    with open (fn, "w") as f:
        #csv_writer = csv.DictWriter(f, fieldnames=columns)
        csv_writer = csv.writer(f, delimiter=args.delimiter)
        csv_writer.writerow(columns)
    return fn

def run():
    ser = createSerialObj(args.port, args.baud, None)
    if ser == None:
        print("no serial port available")
        sys.exit(1)
    
    fn = createNewFile()
    if args.verbose > 2:
        print (f"output file '{fn}'")
    count = 0
    lb = 80
    with open (fn, "a") as f:
        cw = csv.writer(f, delimiter=args.delimiter)
        print(f"writing into '{fn}'")
        data = readSerialData(ser, cw)

        



if __name__ == "__main__":
    run()    