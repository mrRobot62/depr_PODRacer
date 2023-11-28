# PODRacer visualizer

> This python App is a part of the PODRacer Firmware and will be moved in future into a separate git repository

This visualizer app is used to visualize telemetry data which is produced by the PODRacer firmware.
Written in Python with ploltly and dash to visualize the results inside a browser window

Typical telemetry data is
- channel[0] to channel[7] in and out going (mapped in a range from 1000...2000)
- float data (max 8) produced by tasks to store data from pid controllers, kalman filters, ...
- long data (max 8) produced by tasks to store data indivdually content
- data groups (task) and sub groups (e.g. from Group-SDIST SubGroups: TOF-Data and LIDAR data)
- time stamp in milliseconds
- 

> details descriptions please refere to `doc` subfolder

## Using Blackbox data
Not implemented yet

## Using Serial data

### Serial format
``` ```

