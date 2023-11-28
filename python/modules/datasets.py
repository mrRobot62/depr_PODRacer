import pandas               as pd
import numpy                as np

#------------------------------------------------------------------------------------------------------
# Read csv data file
df = pd.read_csv(
#    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/students.csv",
    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/podrdata.csv",
    #dtype={'MS':int, 'TASK':str, 'GROUP':str},
    delimiter=',',
)

df["GROUPING"] = df.TASK + "_" + df.GROUP
df.set_index(["GROUPING"], inplace=True)

idx_time_min = df["TIME"].min()
idx_time_max = df["TIME"].max()

