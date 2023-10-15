#include "OpticalFlow.h"


OpticalFlow::OpticalFlow(uint8_t taskID, SLog *log, uint8_t cs_pin) : TaskAbstract(taskID, log) {
    _cs = cs_pin;
    logger->info("OpticalFlow initialized");
  }


  bool OpticalFlow::begin(Receiver *receiver) {
    flow = new Bitcraze_PMW3901(_cs);
    _recv = receiver;
    if (_recv == nullptr) {
      logger->error("OpticalFlow:: no receiver object available");
      return false;
    }
    if (flow == nullptr) {
      logger->error("PMW3901 not initialized");
      return false;
    }

    slip2RollAxis = 0.0;
    slip2RollAxis = 0.0;

    rawX = 0;
    rawY = 0;
    setPointSlipX = 0.0;
    setPointSlipY = 0.0;

    flowCounter = 0;
    //
    // PID behaviour
    // rawInX represent the input value which got by sensor
    // The PID controller try to adjust to zero for X/Y (0 means no slip/movements)
    //
    pidX = new PID(&slipAdjX, &slip2RollAxis, &setPointSlipX, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, P_ON_E, DIRECT);
    pidY = new PID(&slipAdjY, &slip2PitchAxis, &setPointSlipY, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, P_ON_E, DIRECT);


    if (pidX == nullptr || pidY == nullptr) {
      logger->error("PID controller not initialized");
      return false;
    }
    // PID pidX(&moveX, &deltaXpitch, &deltaXsetpoint, KpTune, KiTune, KdTune, P_ON_E, DIRECT);
    // PID yPid(&moveY, &deltaYroll, &deltaYsetpoint, KpTune, KiTune, KdTune, P_ON_E, DIRECT);

    if (flow->begin() == false) {
      logger->error("Initialization flow sensor failed");
      return false;
    }
    logger->info("ObticalFlow object ready");
    return true;
  }

  /**
    update TData for this OpticalFlow sensor

  **/
  void OpticalFlow::update(void)  {
    #if defined(LOG_TASK_OPTICALFLOW)

      sprintf(buffer,"R:%D, P:%d, Y:%d, T:%d, H:%D, cnt:%d",
        _recv->getData(ROLL),
        _recv->getData(PITCH),
        _recv->getData(THROTTLE),
        _recv->getData(YAW),
        _recv->getData(HOVER),
        flowCounter
      );
      logger->debug(buffer);
    #endif 

    /** if one gimbal is not centered, no optical flow measurement is needed **/
    if (  ((_recv->isGimbalCentered(ROLL) == false) ||
          (_recv->isGimbalCentered(PITCH) == false) ||
          (_recv->isGimbalCentered(YAW) == false))
     ) {
      #if defined(LOG_TASK_OPTICALFLOW)
        logger->info("CENTERED (RPY)");
      #endif
      return;
    }

    // count how long the PODRacer slips. If a threshold is reached, system starts with a countermeasure
    flow->readMotionCount(&rawX, &rawY);
    // we just wait some time if a flow was recognizes
    // if value is greater than threshold, we try avoid this drift and set channel values for roll/pitch
    // (BZW only roll/pitch can remove drifting (not yaw!) )
    //if (flowCounter > FLOW_COUNTER_MAX) {
      // flow controll algorithm
      // check flow X/Y, PID-Adjustment(X/Y), set new relative channel values
      // PID use double values and return double. so first cast value form sensor
      // remove noice
      rawX = (isInRange(rawX, -PMW3901_ZERO, PMW3901_ZERO))?0:rawX;
      rawY = (isInRange(rawY, -PMW3901_ZERO, PMW3901_ZERO))?0:rawY;

      // cumulate to roll and pitch axis. 
      slip2RollAxis += (double)rawX;
      slip2PitchAxis += (double)rawY;

      pidY->Compute();
      pidX->Compute();

      //
      // translate x/y to roll/pitch movements
      // forward drift pitch less (podracer tilts slowly backwards)
      // backward drift pitch greater (podracer tilts slowly forward)
      // left drift : roll right
      // reight drift : roll left
      // left forwards pitch less, roll right
      // left backwards pitch greater, roll right
      // right forwards pitch less, roll left
      // right backwards pitch greater, roll right
      

      //_data[ROLL]
      #if defined(LOG_TASK_OPTICALFLOW)
        #if defined(USE_SERIAL_PLOTTER)
          sprintf(buffer, "X:%d,Y:%d,Roll:%f,Pitch:%f", rawX, rawY, slip2RollAxis, slip2PitchAxis);
          logger->simulate(buffer);
        #else
          sprintf(buffer, "X/Y/Roll/Pitch: <%4d,%4d> <%8f,%8f> CNT(%3d)", rawX, rawY, slipX, slipY);
          logger->info(buffer);
        #endif
      #endif
      flowCounter = 0;
      rawX = rawY = 0;
      slip2RollAxis = slip2PitchAxis = 0.0f;
    //}
/*
    if (rawX == 0) slipX = 0.0;
    if (rawY == 0) slipY = 0.0;

    slipX += (double)rawX;
    slipY += (double)rawY;

    if (isInRange(slipX, -SLIP_RANGE, SLIP_RANGE) == false) {
      slipX = 0;
    }

    if (isInRange(slipY, -SLIP_RANGE, SLIP_RANGE) == false) {
      slipY = 0;
    }


    #if defined (LOG_TASK_OPTICALFLOW) || defined (LOG_TASK_ALL)
      #if defined (USE_SERIAL_PLOTTER)
        sprintf(buffer, "rawX:%d, rawY:%d, slipX:%f, slipY:%f, slipAdjX:%f, slipAdjY:%f",
          rawX, rawY,
          slipX, slipY,
          slipAdjX, slipAdjY
        );
        logger->simulate(buffer);
      #elif defined (LOG_TASK_OPTICALFLOW)
        sprintf(buffer, "rX:%d, rY:%d, sX:%f, sY:%f, sAX:%f, sAY:%f",
          rawX, rawY,
          slipX, slipY,
          slipAdjX, slipAdjY
        );
        logger->info(buffer);
      #endif
    #endif 
*/


  
  }
