#include "PID_v1.h"
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
    // depends who the sensor is assembled

    if ((direction[0] == 0) || (direction[0] == 2)) { // NORTH or EAST
      // sensor is assembled in NORTH/SOUTH direction
        pidX = new PID(&slipAdjX, &slip2RollAxis, &setPointSlipX, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, DIRECT);
        pidY = new PID(&slipAdjY, &slip2PitchAxis, &setPointSlipY, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow,DIRECT);
    }
    else if ((direction[0] == 1) || (direction[0] == 3)) {// WEST or SOUTH
      // sensor is assembled in NORTH/SOUTH direction
        pidY = new PID(&slipAdjY, &slip2RollAxis, &setPointSlipY, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, DIRECT);
        pidX = new PID(&slipAdjX, &slip2PitchAxis, &setPointSlipX, kpOpticalFlow, kiOpticalFlow, kdOpticalFlow, DIRECT);
    }

    if (pidX == nullptr || pidY == nullptr) {
      logger->error("PID controller not initialized");
      return false;
    }

    pidX->SetMode(AUTOMATIC);
    pidY->SetMode(AUTOMATIC);
    pidX->SetOutputLimits(-PID_OUTPUT_LIMIT, PID_OUTPUT_LIMIT);
    pidY->SetOutputLimits(-PID_OUTPUT_LIMIT, PID_OUTPUT_LIMIT);
    pidX->SetSampleTime(LOOP_TIME);
    pidY->SetSampleTime(LOOP_TIME);

    if (flow->begin() == false) {
      logger->error("Initialization flow sensor failed");
      return false;
    }

    logger->info("ObticalFlow object ready");
    cnt = 0;
          rawX = 65;
      rawY = 50;
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
    rpy[0] = _recv->isGimbalCentered(ROLL,true);
    rpy[1] = _recv->isGimbalCentered(PITCH,true);
    rpy[2] = _recv->isGimbalCentered(YAW,true);
    
    if ((rpy[0] == 0) || (rpy[1] == 0)) {
//    if ( (_recv->isGimbalCentered(ROLL,true)==false) ||  (_recv->isGimbalCentered(PITCH,true)==false)) {
      #if defined(LOG_TASK_OPTICALFLOW)
        sprintf (buffer, "xx R:%4d - P:%4d - Y:%4d -- RPY:%d-%d-%d -- not centered", 
        _recv->getData(ROLL), 
        _recv->getData(PITCH), 
        _recv->getData(YAW),
        rpy[0],rpy[1],rpy[2]
        );
        logger->info(buffer);
        cnt=0;
        rawX = 100;
        rawY = 100;
      #endif
      _data.updated = false;
      return;
    }

    // nur für test
    if (cnt > 30) return;

    //return;
    // count how long the PODRacer slips. If a threshold is reached, system starts with a countermeasure
    //flow->readMotionCount(&rawX, &rawY);
      // we just wait some time if a flow was recognizes
      // if value is greater than threshold, we try avoid this drift and set channel values for roll/pitch
      // (BZW only roll/pitch can remove drifting (not yaw!) )
      // flow controll algorithm
      // check flow X/Y, PID-Adjustment(X/Y), set new relative channel values
      // PID use double values and return double. so first cast value form sensor
      // remove noice

      // Value hight depends on velocity. As higher velocity as higher value
      rawX = (isInRange(rawX, -PMW3901_ZERO, PMW3901_ZERO))?0:rawX;
      rawY = (isInRange(rawY, -PMW3901_ZERO, PMW3901_ZERO))?0:rawY;

      // adjust sensor direction
      rawX = direction[1] * rawX ;
      rawY = direction[2] * rawY ;

      // cumulate to roll and pitch axis. 
//      slipAdjX += (double)rawXnormalized;
//      slipAdjY += (double)rawYnormalized;
      slipAdjX += (double)rawX;
      slipAdjY += (double)rawY;
             ;

      // note: pidXY result ist stored in slipRollAxis / slipPitchAxis
      pidY->Compute();
      pidX->Compute();
      // based on slipAdjX/Y (cumulated rawX/Y values) the PID controller calculate a 
      // new adjustable value. this value is mapped is in range ob PID-Limits (SetOutputLimits(-PID_OUTPUT_LIMIT, PID_OUTPUT_LIMIT);)

      _data.ch[ROLL]  = (int16_t)slip2RollAxis;
      _data.ch[PITCH] = (int16_t)slip2PitchAxis;


     // _data.ch[ROLL] = (int16_t)(slip2RollAxis * biasRoll);     
     // _data.ch[PITCH] = (int16_t)(slip2PitchAxis * biasPitch);
     //_data.ch[ROLL] = map(_data.ch[ROLL],-100,100, -PID_OUTPUT_LIMIT, PID_OUTPUT_LIMIT);   
      ;
      //_data[ROLL]
      #if defined(LOG_TASK_OPTICALFLOW)
        #if defined(USE_SERIAL_PLOTTER)
          
          sprintf(buffer, "rX:%3d, rY:%3d, Xnorm:%3d, Ynorm:%3d, sRAX:%4.2f, sRAY:%4.2f, Roll:%4d, Pitch:%4d", 
                rawX, rawY, 
                rawXnormalized, rawYnormalized, 
                slip2RollAxis, slip2PitchAxis,
                _data.ch[ROLL], _data.ch[PITCH]);
            /*
            sprintf(buffer, "sAX:%f, sAY:%f, Roll:%f, Pitch:%f, chR:%d, chP:%d",
                    slipAdjX, slipAdjY,
                    slip2RollAxis, slip2PitchAxis, 
                    _data.ch[ROLL], _data.ch[PITCH]
                    );
            */
          logger->simulate(buffer);
        #else
          sprintf(buffer, "X/Y/Roll/Pitch: <%4d,%4d> <%8f,%8f> CNT(%3d)", rawX, rawY, slipX, slipY);
          logger->info(buffer);
        #endif
      #endif

      cnt++;

      flowCounter = 0;
      rawX += (int16_t)slip2RollAxis;
      rawY += (int16_t)slip2PitchAxis;

      slip2RollAxis = slip2PitchAxis = 0.0f;
      slipAdjX = slipAdjY = 0.0f;
      // reset upadate flag
      _data.updated = true;
    //}
  }
