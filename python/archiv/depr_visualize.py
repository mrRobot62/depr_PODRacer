from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import argparse
import sys
import os
from scipy.interpolate import make_interp_spline
import tkinter
import tkinter.messagebox
import customtkinter

customtkinter.set_appearance_mode(Config.tkinter["appearance_mode"]) 
customtkinter.set_default_color_theme(Config.tkinter["default_color_theme"])

parser = argparse.ArgumentParser()
parser.add_argument("--csv", required=True, type=str, help="csv file to scan")
parser.add_argument("--path", type=str, default="./data", help="path to file, default ./data")
parser.add_argument("--delimiter", type=str, default=",", help="delimiter, default comma (,)")

args = parser.parse_args()

fname = os.path.join(args.path, args.csv)

index = count()
idx = 0
columns = ["TIME", "TASK","GROUP","ARMING", "CH_R", "CH_P", "CH_Y", "CH_H", "CH_T","float0","float1", "float2", "float3", "float4", "float5", "float6", "float7","ldata0","ldata1","ldata2","ldata3","ldata4","ldata5","ldata6","ldata7"];
df = pd.read_csv(fname, usecols=columns, dtype={"TIME":'int', "TASK":'string', "GROUP":'string'})
idx = df["TIME"].min()
print("--------------------------------------------------")
print(f"minium time {idx}")
print("--------------------------------------------------")

class App(customtkinter.CTk):
    def __init__(self):
        super().__init__()
        self.cfg = Config
        self.title("PODRacer LogVisualizer")
        self.geometry(Config.tkinter["geometry"])



    def open_file_explorer(self):
        pass

    def change_zoom(self):
        pass


if __name__ == "__main__":
    app = App()
    app.mainloop()



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