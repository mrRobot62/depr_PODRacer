#include "OpticalFlow.h"


OpticalFlow::OpticalFlow(SLog *log, uint8_t cs_pin) : TaskAbstract(log) {
    _cs = cs_pin;
    logger->info("OpticalFlow initialized");

  }


  bool OpticalFlow::begin(void) {
    flow = new Bitcraze_PMW3901(_cs);
    if (flow == nullptr) {
      logger->error("PMW3901 not initialized");
      return false;
    }
    slipX = slipY = slipAdjX = slipAdjY = 0;
    rawX = rawY = 0;
    setPointSlipX = setPointSlipY = 0;


    //
    // PID behaviour
    // rawInX represent the input value which got by sensor
    // set
    pidX = new PID(&slipX, &slipAdjX, &setPointSlipX, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, P_ON_E, DIRECT);
    pidY = new PID(&slipY, &slipAdjY, &setPointSlipY, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, P_ON_E, DIRECT);

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
  void OpticalFlow::update(void) {
    flow->readMotionCount(&rawX, &rawY);
    //sprintf(buffer, "Flow: <%6d,%6d>", rawX, rawY);
    //logger->info(buffer);
    //
    // it's normal, that the PODRacer slip in X/Y directions, but we have to control
    // how far the PODRace slip. If to much, the algorithm try to set new values for roll/pitch/yaw/throttle


    slipX += rawX;
    slipY += rawY;

    //
    // normally the 
    if (isInInterval(slipX, GIMBAL_CENTER_POSITION, MOVE_RANGE ) == false) {
      slipX = 0;
    }

    if (isInInterval(slipY, GIMBAL_CENTER_POSITION, MOVE_RANGE ) == false) {
      slipY = 0;
    }

    // note we set a new slipAdjX/Y value based on the current slipX/Y value
    pidX->Compute();
    pidY->Compute();


    
    #if defined (LOG_TASK_OPTICALFLOW) || defined (LOG_TASK_ALL)
      #if defined (USE_SERIAL_PLOTTER)
        sprintf(buffer, "rawX:%d, rawY:%d, slipX: %d, slipY:%d, slipAdjX:%d, slipAdjY:%d",
          rawX, rawY,
          slipX, slipY,
          slipAdjX, slipAdjY
        );
        logger->simulate(buffer);
      #elif defined (LOG_TASK_OPTICALFLOW)
        sprintf(buffer, "rX:%d, rY:%d, sX:%d, sY:%d, sAX:%d, sAY:%d",
          rawX, rawY,
          slipX, slipY,
          slipAdjX, slipAdjY
        );
        logger->info(buffer);
      #endif
    #endif 

  }
