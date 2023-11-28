#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
PODRacer Visualizer

This is a logviewer more or less compareable with the BetaFlight LogViewer. Biggest difference is, this visualizer
is written in Python and a TKinter based application (customtikinter)

PODRacer fw create dozens of additional commands (channel data) which are received from a SBUS receiver and
send directly to the FlightController.

These new data can be logged (visualized) by this app.

Solution 1:
during test(development) it is possible to send data via serial interface to a host and can be used by the visualizer

Solution 2: (planning)
during flight, telemetry data is stored as blackbox data stored on SDCard. This packed binary data can be visualized by this app

Solution 3: (planning)
extended solution to (1) data transfer via WLAN

base structure idea copied from https://github.com/jxjo/PlanformCreator2/tree/main
"""

import os
import sys
import argparse
import itertools

from pathlib                import Path
from matplotlib.animation   import FuncAnimation
from itertools              import count
from scipy.interpolate      import make_interp_spline
from tkinter                import filedialog
import customtkinter as ctk

import pandas               as pd
import matplotlib.pyplot    as plt
import argparse
import tkinter
import tkinter.messagebox
import customtkinter



sys.path.append(os.path.join(Path(__file__).parent , 'modules'))

from modules.common_utils       import * 
from modules.ui_base            import set_initialWindowSize              
from modules.widgets            import * 
from modules.frames             import *

parser = argparse.ArgumentParser()
parser.add_argument("--csv", required=True, type=str, help="csv file to scan")
index = count()
idx = 0
columns = ["TIME", "TASK","GROUP","ARMING", "CH_R", "CH_P", "CH_Y", "CH_H", "CH_T","float0","float1", "float2", "float3", "float4", "float5", "float6", "float7","ldata0","ldata1","ldata2","ldata3","ldata4","ldata5","ldata6","ldata7"];
#df = pd.read_csv(fname, usecols=columns, dtype={"TIME":'int', "TASK":'string', "GROUP":'string'})
#idx = df["TIME"].min()

AppName    = "PODRacer LogVisualizer"
AppVersion = "1.0.1"

def fireEvent(ctk_root : ctk.CTkToplevel, eventType): 
    """ fire event for the current ctk_root toplevel widget """
    if not ctk_root is None: 
        ctk_root.event_generate (eventType) 

class App(customtkinter.CTk):
    iter=itertools.count()
    row = 0
    def __init__(self, paramFile):
        super().__init__()
        self.paramFile = paramFile
        self.settings = Settings()
        self.title(f"{AppName} V{AppVersion}")
        set_initialWindowSize(self, widthFrac=0.92, heightFrac=0.8)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=1)

        #self.channelFrame = ChannelFrame(self, None)
        #self.channelFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")
        #self.upperFrame = UpperFrame(self, None)
        #self.upperFrame.grid(row=0, column=1, padx=10, pady=(10, 0), sticky="nsw")
        #self.floatFrame = FloatFrame(self, None)
        #self.floatFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")
        #self.longFrame = LongFrame(self, None)
        #self.longFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")

        self.channelFrame = SwitchFrame(self, None, ['ROLL', 'PIT','YAW','HOV','THR','AUX2','AUX3'],"CHANNELS")
        self.channelFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")
        self.floatFrame = SwitchFrame(self, None, ['FLT1', 'FLT2','FLT3','FLT4','FLT5','FLT6','FLT7', 'FLT8'],"FLOATS")
        self.floatFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")
        self.longFrame = SwitchFrame(self, None, ['LNG1', 'LNG2','LNG3','LNG4','LNG5','LNG6','LNG7', 'LNG8'],"FLOATS")
        self.longFrame.grid(row=next(self.iter), column=0, padx=10, pady=(10, 0), sticky="nsw")


        self.row = next(self.iter)
        print (self.row)
        self.fileBtn = customtkinter.CTkButton(self, text="LOAD", command=self.load_callback)
        self.fileBtn.grid(row=self.row, column=1, padx=10, pady=10, sticky="ew")
        self.runBtn = customtkinter.CTkButton(self, text="RUN", command=self.run_callback, fg_color="green")
        self.runBtn.grid(row=self.row, column=2, padx=10, pady=10, sticky="ew")
        self.resetBtn = customtkinter.CTkButton(self, text="RESET", command=self.reset_callback, fg_color="orange")
        self.resetBtn.grid(row=self.row, column=3, padx=10, pady=10, sticky="ew")

    def load_callback(self):
        print("load file dialog")

    def run_callback(self):
        print("run pressed")        

    def reset_callback(self):
        print("reset pressed")        

    def open_file_explorer(self):
        pass

    def change_zoom(self):
        pass

#--------------------------------------------


#--------------------------------------------





if __name__ == "__main__":
    just_fix_windows_console()
    InfoMsg(f"Starting {AppName}....")
    Settings.belongTo (__file__, msg=True)
    ctk.set_appearance_mode    (Settings().get('appearance_mode', default='System'))   # Modes:  "System" (standard), "Dark", "Light"
    ctk.set_default_color_theme(Settings().get('color_theme', default='blue'))         # Themes: "blue" (standard), "green", "dark-blue"
    scaling = Settings().get('widget_scaling', default=1.0)
    if scaling != 1.0: 
        ctk.set_widget_scaling(scaling)  # widget dimensions and text size
        NoteMsg ("Font size is scaled to %.2f" %scaling)
    scaling = Settings().get('window_scaling', default=1.0)
    if scaling != 1.0: 
        ctk.set_window_scaling(scaling)  # scaling of window
        NoteMsg ("Window size is scaled to %.2f" %scaling)

    # paramter file as argument?  

    parmFile = ''
    parser = argparse.ArgumentParser(prog=AppName, description='Visualize PODRacer logfiles')
    parser.add_argument("--paramfile", type=str, help="Paramter file .pc2")
    args = parser.parse_args()
    if args.paramfile: 
        parmFile = args.paramfile
    else: 
        #parmFile = Settings().get('lastOpenend', default=".\\examples\\vjx.glide\\VJX.glide.pc2") 
        parmFile = Settings().get('lastOpenend', default=None)



    if parmFile and not os.path.isfile (parmFile):
            ErrorMsg ("Parameter file '%s' doesn't exist" %parmFile )
            Settings().set('lastOpenend', None) 
            Settings().set('lastWindowSize', "{1100}x{800}")
            Settings().set('appearance_mode', "System")
            Settings().set('color_theme', "blue")
            Settings().set('widget_scaling', 1.0)
            parmFile = None

    myApp = App(parmFile)
    myApp.mainloop()



""" 
def animate(i):
    idx = next(index)
    df = pd.read_csv(fname, usecols=columns, dtype={"TIME":'int', "TASK":'string', "GROUP":'string'}
    min = df["TIME"].min() 
    max = df["TIME"].max() 
    timeR = df["TIME"]
    spline = make_interp_spline()
    x = df["TIME"]
    y1 = df["CH_H"]
    plt.cla()
    plt.plot(x, y1)

#    plt.xlim(left=max(0, idx-50 ), right=idx+50)
    #plt.xlim(left=idx-75, right=idx)
    plt.figure

ani = FuncAnimation(plt.gcf(), animate, interval=15)

plt.tight_layout()
plt.show()

 """