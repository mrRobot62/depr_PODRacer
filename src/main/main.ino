/**

Grundlegende Idee

Es wird eine Subsumption Vorgehensweise implementiert. Im Hintergrund läuft
einer cooperativer Scheduler 

Subsumption
Wiederkehrende Funktionen werden als TASKS definiert.
Alle tasks werden in einer "Mixer-Funktion" ausgewertet und priorisiert

Im Prinzip wird die Ausgabe eines geringer priorisierten Tasks durch den nächst
höheren Task überschrieben

Beispiel
Task1 = CRUISE (der PodRacer fliegt einfach geradeaus und hält die Höhe)
Task2 = MovementControl (kontrolliert den OpticalFlowSensor)
Task3 = SurfaceDistance1 (kontrolliert die Höhe über Grund mit dem TFMiniLidar)
Task4 = SurfaceDistance2 (kontrolliert die Höhe über Grund mit dem VL53L1)
Task5 = Receiver - kontrolliert den Empfang von Daten und die Weiterleitung an den FC

Mixer (Priorisierung)


**/

// Include CoopTask since we want to manage multiple tasks.
#include <CoopTaskBase.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <BasicCoopTask.h>
#include <assert.h>
#include "Receiver.h"
#include "Mixer.h"
#include "OpticalFlow.h"
#include "SurfaceDistance.h"
#include "SimpleLog.cpp"
#include "Hover.h"
#include "Steering.h"
#include "BlinkPattern.h"
#include "Blackbox.h"
#include "constants.h"

char * _tname = "MAIN";
int taskToken = 1;
bool showDisarmed = true;

SLog        logger(&Serial, 115200, LOGLEVEL);

Blackbox    bb(&logger, BLACKBOX_CS_PIN);

CoopTask<void>* blinkpatternTask = nullptr;
CoopTask<void>* receiverCtrlTask = nullptr;
CoopTask<void>* movementCtrlTask = nullptr;
CoopTask<void>* hoverCtrlTask = nullptr;
CoopTask<void>* steeringCtrlTask = nullptr;
CoopTask<void>* surfaceDistCtrlTask = nullptr;
CoopTask<void>* mixerCtrlTask = nullptr;


//CoopTask<void>* blackboxCtrlTask = nullptr;

/** 
This semaphore is used to avoid concurrent access to the receives data array during updateing this array
Receiver.cpp set and reset this semaphore. is set, other task can not read the internal data array via getData()
**/
CoopSemaphore taskSema(1, 1);
HardwareSerial lidarSerial(1);      // Parameter 1 => Serial1.


BlinkPattern blinkP(TASK_HB, &logger);
Hover hover(TASK_HOVER, &logger,&bb);
SurfaceDistance distance(TASK_SURFACEDISTANCE, &logger, &lidarSerial, &bb);
OpticalFlow flow(TASK_OPTICALFLOW, &logger, PIN_PMW3901, &bb);
Steering steering(TASK_STEERING, &logger, &bb);
Receiver receiver(TASK_RECEIVER, &logger, &Serial2, 16, 17, true, &bb);
Mixer mixer(TASK_MIXER, &logger, &bb);

//Blackbox bb(TASK_MIXER, &logger, CS_PIN);

// some messages should only be print one time
// if true, print otherweise do not print again
bool log_once_disarmed = true;


//Receiver *receiver;
uint8_t blink_pattern = 0;

// Task: HeartBeat (1 second)
void BlinkPatternFunction() {
  blinkP.begin(&blink_pattern);
  char buffer[100];
  for(;;) {
    blink_pattern = 0;
    if (receiver.isPreventArming()) {
      blink_pattern = PATTERN_PREVENTARMING;
    } else if (!receiver.isArmed()) {
      blink_pattern = PATTERN_DISARMED;
    }
    blinkP.update(blink_pattern);
    yield();
  }
}

// Task: use PMW3901 optical flow sensor
void MovementControlFunction() {
//  taskSema.wait();

  unsigned long lastMillis = millis();
  int16_t deltaX,deltaY;
  if (flow.begin(&receiver) == false) {
    logger.error("MAIN : can't start OpticalFlow", _tname);
    return;
  }
  for(;;) {
    if (!flow.hasError()) {
      //Serial.println("run MovementControlFunction");
      flow.update();
      //flow.readMotionCount(&deltaX, &deltaY);
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      }
      delay(LOOP_TIME);
    }
  }
}


// Primary Task Hovering is the base functionality of the complete PODRacer System
// 
void HoverControlFunction() {
  unsigned long lastMillis = millis();
  if (hover.begin(&receiver) == false ) {
    logger.error("MAIN : can't start Hover object", _tname);
    return;
  }
  for(;;) {
    //Serial.println("HoverControlFunction");
    if (!hover.hasError()) {
      #if defined(LOG_TASK_MIXER_HOVER)
        logger.debug("hover.update()");
      #endif
      hover.update();
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      }
    }
    else {
      logger.error("HOVER return error", _tname);
    }
  }
  delay(LOOP_TIME);
}

// Primary Task Hovering is the base functionality of the complete PODRacer System
// 

// Steering
// 
void SteeringControlFunction() {

  unsigned long lastMillis = millis();
  if (steering.begin(&receiver) == false ) {
    logger.error("MAIN : can't start Steering object", _tname);
    return;
  }

  for(;;) {
    if (!steering.hasError()) {
      //steering.update();
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      }
      delay(LOOP_TIME);
    }
  }

}

// 
void SurfaceDistanceControlFunction() {
  unsigned long lastMillis = millis();
    if (distance.begin(&receiver) == false ) {
      logger.error("MAIN : can't start SurfaceDistance object", _tname);
      return;
    }
  for(;;) {
    if (!distance.hasError()) {
      distance.update();
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      }
      delay(LOOP_TIME);
    }
    else {
      yield();
    }

  }
}

// Task: Mixer 
void MixerControlFunction() {
  char buffer[100];
  unsigned long lastMillis = millis();
  if (&receiver) {
    if (!mixer.begin(&receiver)) {
      logger.error("MAIN : can't start mixer object", _tname);
    }
  }
  else {
    logger.error("mixer.begin() - no receiver object", _tname);
  }
  bool updated = false;
  for(;;) {
    updated=false;
    if (receiver.isArmed()) {
      if (hover.isUpdated()) {/*Serial.println("MIXER(HOVER)");*/ mixer.update(&hover); updated=true;}
      if (distance.isUpdated()) {/*Serial.println("MIXER(SDIST)");*/ mixer.update(&distance); updated=true;}  
      if (steering.isUpdated()) {mixer.update(&steering); updated=true;} 
      if (flow.isUpdated()) {mixer.update(&flow); updated=true;}  
      // if nothing above was updated, than do an explicit update()
      if (!updated) {mixer.update();}
      log_once_disarmed = true;
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      } else {
        delay(LOOP_TIME);
      }


    }
    else {
      if (log_once_disarmed) {
        logger.warn("\n\n********************\nDISARMED\n********************\n", _tname);
        log_once_disarmed = false;
      }
    }
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);

  }
}


// Task: control SBUS activites (Receiver -> EPS32 -> FlightController)
void ReceiverControlFunction() {
  unsigned long lastMillis = millis();
  if (!receiver.begin()) {
    logger.error("MAIN : can't start Receiver object", _tname);
    return;
  }
  char buffer[100];
  for(;;) {
    if (!receiver.hasError()) {
      /** read latest hardware-receiver data into SDATA struct **/
      receiver.update();
      if ((millis() - lastMillis) > LOOP_TIME) {
        lastMillis = millis();
        yield();
      }
      delay(LOOP_TIME);
    }
    else {
      sprintf(buffer, "Receiver has error 0b%s", logger.getBinary(receiver.getError()));
      //logger.error(buffer, _tname);
    }
  }
}


bool DelayCoopFunction(int ms) {
  return true;
}

bool SleepCoopFunction() {

  //Serial.println("run SleepCoopFunction");
  Serial.flush();
  delay(LOOP_TIME);
  if (blinkpatternTask) blinkpatternTask->wakeup();
  if (receiverCtrlTask) receiverCtrlTask->wakeup();
  if (hoverCtrlTask) hoverCtrlTask->wakeup();
  if (steeringCtrlTask) steeringCtrlTask->wakeup();
  if (movementCtrlTask) movementCtrlTask->wakeup();
  if (surfaceDistCtrlTask) surfaceDistCtrlTask->wakeup();
  if (mixerCtrlTask) mixerCtrlTask->wakeup();
  //if (blackboxCtrlTask) blackboxCtrlTask->wakeup();

  #if defined (LOG_TASK_ALL)
    logger->debug ("SleepCoopFunction wakeup tasks done", _tname);
  #endif
  return true;

}

void setup() {
  while (!Serial);
  delay(100);

  #if !defined(ESP32)
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("not running on ESP32 device");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    assert(0);
  #endif

  //Serial.println("setup.....");
  logger.info("setup...", _tname);
  //<todo> should be moved if armed=true
  bb.begin();

  char buffer[30];
  sprintf(buffer, "PODRacer-FWVersion: %s", bb.FWVersioin());
  logger.info(buffer, _tname);

  //-----------------------------------------
  logger.info("create tasks....", _tname);

  blinkpatternTask = new CoopTask<void>(F("BLINK"),BlinkPatternFunction);
  receiverCtrlTask = new CoopTask<void>(F("RECV"),ReceiverControlFunction);
  steeringCtrlTask = new CoopTask<void>(F("STEER"),SteeringControlFunction);
  movementCtrlTask = new CoopTask<void>(F("OFLOW"), MovementControlFunction);
  hoverCtrlTask = new CoopTask<void>(F("HOVER"), HoverControlFunction);
  surfaceDistCtrlTask= new CoopTask<void>(F("SURFD"), SurfaceDistanceControlFunction);
  mixerCtrlTask = new CoopTask<void>(F("MIXER"),MixerControlFunction);
  logger.info("all tasks ready", _tname);

  //-----------------------------------------
  logger.info("schedule tasks", _tname);
  runCoopTasks(nullptr, nullptr, SleepCoopFunction);

  blinkpatternTask->scheduleTask();
  receiverCtrlTask->scheduleTask();
  movementCtrlTask->scheduleTask();
  surfaceDistCtrlTask->scheduleTask();
  hoverCtrlTask->scheduleTask();
  steeringCtrlTask->scheduleTask();
  mixerCtrlTask->scheduleTask();
  logger.info("all tasks scheduled", _tname);

}



void loop() {
  // put your main code here, to run repeatedly:
  //runCoopTasks(nullptr, DelayCoopFunction, SleepCoopFunction);
  runCoopTasks(nullptr, nullptr, nullptr);
  //Serial.println("running tasks....");
  yield();
}
