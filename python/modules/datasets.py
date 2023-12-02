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


#
# below datasets are subsets from the main dataframe
#
df_floats = df.filter([
    'TIME','TASK','GROUP',
    'float0','float1','float2','float3',
    'float4','float5','float6','float7'
], axis=1)

df_longs = df.filter([
    'TIME','TASK','GROUP',
    'ldata0','ldata1','ldata2','ldata3',
    'ldata4','ldata5','ldata6','ldata7'
], axis=1)


df_pids = df.filter([
    'TIME','TASK','GROUP',
    'pidRoll','pidPitch','pidYaw','pidThrust'
], axis=1)

