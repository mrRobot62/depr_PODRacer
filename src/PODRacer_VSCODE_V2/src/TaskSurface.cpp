#include "TaskSurface.h"

TaskSurface::TaskSurface(SLog *log, char *name, uint8_t taskID, CoopSemaphore *taskSema, HardwareSerial *bus) 
  : Task(log, name, taskID, taskSema) {
    this->bus = bus;
}

void TaskSurface::begin(bool allowLog) {
  sprintf(buffer, "begin() - task ready - allowLogging: %d", allowLog);
  log->info(buffer, true, name);
  resetTaskData();
  sprintf(buffer, "reset surface buffer: FW: %s, TaskID:%d, Armed:%d, ch[HOVERING]:%d, ",
          bbd->data.fwversion,
          bbd->data.task_id,
          bbd->data.is_armed,
          bbd->data.ch[HOVERING]);
  log->info(buffer, true, _tname);  

  pidTOF = new PID(&tofSKFValue, &tofPIDAdjValue, &sdistSetPoint,kpSDIST,kiSDIST,kdSDIST, DIRECT);
  pidLIDAR = new PID(&lidarSKFValue, &lidarPIDAdjValue, &sdistSetPoint,kpSDIST,kiSDIST,kdSDIST, DIRECT);
  skfToF = new SimpleKalmanFilter(skfeMea, skfeEst, skfE);
  skfLidar = new SimpleKalmanFilter(skfeMea, skfeEst, skfE);

  if (pidTOF == nullptr || pidLIDAR == nullptr) {
    log->error("PID controller not initialized", name);
    internal_error_occured = true;
    internal_error_code = 0x01;
    setInternalError(this->_id, ERROR_TASK_PID1);
    return;  
  }

  if (skfToF == nullptr || skfLidar == nullptr) {
    log->error("KalmanFilter not initialized", name);
    setInternalError(this->_id, ERROR_TASK_SKF1);
    return;
  }

  // --------------------------------------
  // initializing TOF-Sensor
  // --------------------------------------  
  #ifndef SDIST_IGNORE_TOF_SENSOR {
      #if defined(USE_SDIST_VL53L0)
        tof = new VL53L0X();
      #elif defined(USE_SDIST_VL53L1)
        tof = new VL53L1X();
      #endif

    if (tof == nullptr) {
      log->error("VL53L0/1X not intialized", name);
      setInternalError(this->_id, ERROR_TASK_TOF1);
      return;
    }
    Wire.begin();
    if (!tof->init()) {
      log->error("failed to detect and initalize TOF-Sensor", name);
      internal_error_occured = true;
      setInternalError(this->_id, ERROR_TASK_TOF2);
      return;  
    }
    tof->setTimeout(500);
    tof->startContinuous(SDIST_CONT_SCANS_MS);

    pidTOF->SetMode(AUTOMATIC);
    pidTOF->SetOutputLimits(-SDIST_PID_OUTPUT_LIMIT, SDIST_PID_OUTPUT_LIMIT);   // -100...+100
    pidTOF->SetSampleTime(LOOP_TIME);

    log->info("TOF-Sensor complete initialized...",allowLog, name);
    ignore_sensor[USE_TOF_SENSOR] = false;
  #else
    log->warn("TOF-Sensor not available based on SDIST_IGNORE_TOF_SENSOR",name);
    ignore_sensor[USE_TOF_SENSOR] = true;
  #endif
    // --------------------------------------
    // initializing LIDAR-Sensor (TFMini)
    // --------------------------------------
    lidar = new TFMPlus();
    if (lidar == nullptr) {
      log->error("TFMini-Lidar sensor not created");
      setInternalError(this->_id, ERROR_TASK_LIDAR1) ;
      return;  
    }

  /*
    RX1_PIN and TX1_PIN occupy the IO ports for the internal flash memory. 
    When flashing the program, GPIO2 must not be connected to the TFMini.
  */
  #ifndef SDIST_IGNORE_TOF_SENSOR {

    bus->begin(BAUD_115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
    Serial.println("try lidar->begin()");
    if (lidar->begin(bus) == false) {
      log->error("TFMini-Lidar initialization failed", name);
      setInternalError(this->_id, ERROR_TASK_LIDAR2) ;
      return;
    }

    this->setUpdated(false);
    // reset error flag
    this->setInternalError(0, this->_id);

    log->info("LIDAR-Sensor complete initialized...",allowLog, name);
    ignore_sensor[USE_LIDAR_SENSOR] = false;
  #else
    log->warn("LIDAR-Sensor not available based on SDIST_IGNORE_LIDAR_SENSOR", name);
    ignore_sensor[USE_LIDAR_SENSOR] = true;
  #endif
}

/**
  SurfaceDistance use two different sensors (TFMini & VL53Lxy). The TFMini is in front of the
  PODRacer, the other one in the back.

  Theoretically both sensors should measure the same distance, but obviosluy that's not a typical
  szenario.

  Inside this methode, we calculate a average between both values and put them together into the 
  PIDController to 

  Important Notes to understand Visualization-Output (and storing data into task data structure)

  General
  ch[0..8]  stores only a releative deviation from the actual mean value (1500)
  TOF
  fdata [0] PID output
  fdata [1] HOVER Value - this represent the relativ value in the channel (as doublt)
  fdata [2] PID Setpoint
  fdata [3] Kalman filter output value

  ldata [0] TOF raw value 
  ldata [1] HOVER value (same as ch[HOVER])
  ldata [2] 0 - not used
  ldata [3] 0 - not used

  LIDAR
  fdata [4] PID output
  fdata [5] HOVER Value - this represent the relativ value in the channel
  fdata [6] PID Setpoint
  fdata [7] Kalman filter output value
  
**/
void TaskSurface::update(bool armed, bool allowLog) {
  if (armed) {
    resetTaskData();
    if (ignore_sensor[USE_TOF_SENSOR]==true) {
      log->once_warn(&log_once_mask, LOG_ONCE_SDIST_TOF_WARN3, "TOF-Sensor ignored!", name);
    }
    if (ignore_sensor[USE_LIDAR_SENSOR]==true) {
      log->once_warn(&log_once_mask, LOG_ONCE_SDIST_LIDAR_WARN3, "LIDAR-Sensor ignored!", name);
    }
    log->once_warn(&log_once_mask, LOG_ONCE_DATA1_BIT, "PODRacer armed", name);
    bbd->data.start_millis = millis();
    //bbd->data.ch[ROLL] = HOVER_ROLL;
    //bbd->data.ch[PITCH] = HOVER_PITCH;
    bbd->data.ch[HOVERING] = 0;           // reset to 0, task will create a delta value for hovering
    bbd->data.ch[THRUST] = HOVER_THRUST;
    bbd->data.ch[YAW] = HOVER_YAW;
    bbd->data.ch[AUX2] = GIMBAL_MIN;
    bbd->data.ch[AUX3] = GIMBAL_MIN;
    bbd->data.const_hover[0] = SDIST_MINIMAL_HEIGHT;
    bbd->data.const_hover[1] = SDIST_MIN_DISTANCE;
    bbd->data.const_hover[2] = SDIST_MAX_DISTANCE;
    bbd->data.updated = true;
    bb->update(bbd);

    if (ignore_sensor[USE_TOF_SENSOR]==false) {
      if (ignore_sensor[USE_LIDAR_SENSOR] == false) {
        if (lidar->getData(lidarDist, lidarFlux, lidarTemp)) { // Get data from the device.
          if (LOGLEVEL > 3) {
            sprintf(buffer, "Dist(TFMini):%04icm\tFlux:%05i\tTemp:%03iC°", lidarDist, lidarFlux, lidarTemp);
            log->info(buffer, _tname);
          }
        }
    }
    else {
      // if we do not have a LIDAR-Sensor, we get default data as mock data
      getMockedData(bbd, 0);
      lidarDist = bbd->data.ldata[SDIST_LDATA_LIDAR_RAW];
    }
    lidarDist *= 10;
    lidarDist = constrain(lidarDist, SDIST_COND_MIN_VALUE, SDIST_COND_MAX_VALUE);

    if (ignore_sensor[USE_TOF_SENSOR] == false) {
      tofMm = tof->readRangeContinuousMillimeters();
      tofMm = constrain(tofMm, SDIST_COND_MIN_VALUE, SDIST_COND_MAX_VALUE);
    }
    else {
      // if no TOF-sensor used, set raw value to the defined setPoint
      tofMm = (long)sdistSetPoint;  
    }
    // if something happens, we set channel to zero (no additional load for HOVERING)
    bbd->data.ldata[SDIST_LDATA_LIDAR_RAW] = (long)lidarDist;          // store raw value
    bbd->data.ldata[SDIST_LDATA_TOF_RAW] = (long)tofMm;                // store raw value

    if (tofMm < SDIST_MINIMAL_HEIGHT && lidarDist < SDIST_MINIMAL_HEIGHT) {
      // if minimal height is not reached no height calculation is needed
      sprintf(buffer, "<<<-- HEIGHT TO LOW (%4imm,%4imm,%imm, %4imm)\tTOF:%5imm\tLIDAR:%5imm --->>>", 
        (long)SDIST_MINIMAL_HEIGHT,
        (long)SDIST_MIN_DISTANCE,
        (long)sdistSetPoint,
        (long)SDIST_MAX_DISTANCE,
        //_bbd.data.ldata[SDIST_LDATA_TOF_RAW]
        tofMm,
        lidarDist
      );
      log->info(buffer, allowLog, name);
      return;
    }    

    //
    // If PODRacer is higher as MINIMAL_HEIGHT, than we use a PID-Controller to
    // put it into the correct height
    tofMm = constrain(tofMm, SDIST_MINIMAL_HEIGHT, SDIST_COND_MAX_VALUE);

    tofRawValue = (double)tofMm;          // convert to double
    lidarRawValue = (double)lidarDist;     // convert to double

    // raw values are filtered to remove jitter
    // this value is used by KalmanFilter to smoothing the real output (for both sensors in its own Kalmanfilter )
    tofSKFValue = skfToF->updateEstimate(tofRawValue);
    lidarSKFValue = skfLidar->updateEstimate(lidarRawValue);
    tofPIDAdjValue = 0.0;
    lidarPIDAdjValue = 0.0;
    // PID Controll save adjusted value into tofPIDAdjValue
    // Input is the filtered value from kalman and output is saved into tofPIDAdjValue

    // Parameters for PID(&tofSKFValue, &tofPIDAdjValue, &tofSetPoint, kpTOF, kiTOF, kdTOF, DIRECT);
    pidTOF->Compute();
    pidLIDAR->Compute();


    bbd->data.fdata[SDIST_FDATA_TOF_PID] = tofPIDAdjValue;                          // store adjusted (OUT) value from PID
    bbd->data.fdata[SDIST_FDATA_TOF_HOVER] = (tofPIDAdjValue * SDIST_BIAS);         // adjust a little bit this value with a bias 
    bbd->data.fdata[SDIST_FDATA_TOF_SETPOINT] = sdistSetPoint;                      // set point for TOF-PID
    bbd->data.fdata[SDIST_FDATA_TOF_SKFVALUE] = tofSKFValue;                        // store calculated SKF value from raw input data
    
    bbd->data.fdata[SDIST_FDATA_LIDAR_PID] = lidarPIDAdjValue;                      // store adjusted (OUT) value from PID
    bbd->data.fdata[SDIST_FDATA_LIDAR_HOVER] = (lidarPIDAdjValue * SDIST_BIAS);     // adjust a little bit this value with a bias 
    bbd->data.fdata[SDIST_FDATA_LIDAR_SETPOINT] = sdistSetPoint;                    // set point for TOF-PID
    bbd->data.fdata[SDIST_FDATA_LIDAR_SKFVALUE] = lidarSKFValue;                    // store calculated SKF value from raw input data


    bbd->data.ldata[SDIST_LDATA_TOF_HOVER] = (int16_t)bbd->data.fdata[SDIST_FDATA_TOF_HOVER];           // store hovering 
    bbd->data.ldata[SDIST_LDATA_LIDAR_HOVER] = (int16_t)bbd->data.fdata[SDIST_FDATA_LIDAR_HOVER];       // store hovering 

    hoverValue = max(bbd->data.ldata[SDIST_LDATA_TOF_HOVER], bbd->data.ldata[SDIST_LDATA_LIDAR_HOVER]);

    bbd->data.ch[HOVERING] = hoverValue;

    sprintf(buffer, "TOF  (RAW|SKF|PID|SET|HOV): %4imm|%.2f|%f|%.2f|%4i",
      bbd->data.ldata[SDIST_LDATA_TOF_RAW],
      tofSKFValue,
      bbd->data.fdata[SDIST_FDATA_TOF_PID],
      sdistSetPoint,
      bbd->data.fdata[SDIST_LDATA_TOF_HOVER]
    );
    log->info(buffer, allowLog, name, "TOF");


    sprintf(buffer, "LIDAR(RAW|SKF|PID|SET|HOV): %4imm|%.2f|%f|%.2f|%4i",
      bbd->data.ldata[SDIST_LDATA_LIDAR_RAW],
      lidarSKFValue,
      bbd->data.fdata[SDIST_FDATA_TOF_PID],
      sdistSetPoint,
      bbd->data.fdata[SDIST_LDATA_LIDAR_HOVER]    
    );
    log->info(buffer, allowLog, name, "LID");

    bbd->data.end_millis = millis();
    this->setUpdated();
    log->data(bbd, allowLog, name, "UPD");
  }
  else {
    log->once_warn(&log_once_mask, LOG_ONCE_WARN0_BIT,"PODRacer disarmed", name);
  }
}

