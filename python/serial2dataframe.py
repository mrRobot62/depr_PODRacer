import time
from datetime import datetime
import serial
import argparse
import sys
import os
import pandas as pd


columns = [
            # grouping information
            "TIME", "TASK","GROUP",
            # channels 
           "CH_R", "CH_P", "CH_Y", "CH_H", "CH_T",
           "ARMING","AUX2","AUX3",
           # floats & long values, used by tasks
           "float0","float1", "float2", "float3", "float4", "float5", "float6", "float7",
           "long0","long1","long2","long3","long4","long5","long6","long7",
           # pid values used by tasks
           "pidRoll","pidPitch","pidYaw","pidThrust","pidHover",
           # constants.....
           "HOVER_MINIMAL_HEIGHT", "HOVER_MIN_DISTANCE", "HOVER_MAX_DISTANCE",
           # CRC-16Bit, FW send 8 Bytes 4 Padding-Bytes and 4 bytes CRC
           "CRC"
           ]



parser = argparse.ArgumentParser()

parser.add_argument("--port", type=str, help="insert your serial port")
parser.add_argument("--baud", type=int, default=115200, help="Baud rate, default=115200")
parser.add_argument("--tout", type=int, default=5, help="Timout in seconds, default=5")
parser.add_argument("--opath", type=str, default="./data", help="output path, default is ./data")
parser.add_argument("--delimiter", type=str, default=",", help="delimiter, default is comma (,)")
parser.add_argument("--use_ts", action="store_true", help="if set, use a timestamp in file name")
parser.add_argument("-v", "--verbose", action="count", default=1, help="verbosity output -v -vv -vvv")

args = parser.parse_args()

def getFileName(path="./"):
    dt = datetime.fromtimestamp(time.time())
    ts = ""
    if args.use_ts:
        ts = dt.strftime("_%Y%m%d_%H%M%S")
    fn = "live_data{TS}.csv"
    f = os.path.join(path, fn.format(TS=ts))
    return f

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
    s = None
    try:
        s = serial.Serial(port=port,baudrate=baud, timeout=tout)
        s.isOpen()
        print("SerialPort open");
    except IOError as err:
        print ("Port already in use. Try to close ...")
        if s is None:
            print (f"problem to create serial object on '{port}' with '{baud}' baudrate")
            exit(1)
        s.close()
        print ("Port closed, try to reopen...")
        s.open()
    except Exception as err:
        print (err)
        print (f"problem to create serial object on '{port}' with '{baud}' baudrate")
        return None
    return s

def readSerialData(ser:serial, df:pd.DataFrame):
    """

    """
    try:
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
            df.loc[len(df)] = raw[1:]       # ignore sync byte FEEF
            if (lines % 100 == 0):
                if (lb % 10 == 0) and (lb > 0): 
                    print("")
                    lb = 0
                print(f"{lines:>6} ",end="")
                lb += 1
            lines += 1
            if lines > 5000:
                timeout = True 

    except IOError as err:
        print (f"SerialIOError '{str(err)}'")
    except Exception as err:
        print (f"General exception '{str(err)}'")
    print("saving data...")
    fn = getFileName("./")
    df.to_csv(fn)

def run():
    ser = createSerialObj(args.port, args.baud, None)
    if ser == None:
        print("no serial port available")
        sys.exit(1)
    df = pd.DataFrame(columns=columns)

    if args.verbose > 2:
        print (f"Dataframe created with {columns}")
    count = 0
    lb = 80
    data = readSerialData(ser, df)


if __name__ == "__main__":
    run()    