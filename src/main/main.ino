/**

Grundlegende Idee

Es wird eine Subsumption Vorgehensweise implementiert. Im Hintergrund läuft
einer cooperativer Scheduler 

Subsumption
Wiederkehrende Funktionen werden als TASKS definiert.
Alle tasks werden in einer "arbitrate-Funktion" ausgewertet und priorisiert

Im Prinzip wird die Ausgabe eines geringer priorisierten Tasks durch den nächst
höheren Task überschrieben

Beispiel
Task1 = CRUISE (der PodRacer fliegt einfach geradeaus und hält die Höhe)
Task2 = MovementControl (kontrolliert den OpticalFlowSensor)
Task3 = SurfaceDistance1 (kontrolliert die Höhe über Grund mit dem TFMiniLidar)
Task4 = SurfaceDistance2 (kontrolliert die Höhe über Grund mit dem VL53L1)
Task5 = Receiver - kontrolliert den Empfang von Daten und die Weiterleitung an den FC

Arbitrate (Priorisierung)


**/

// Include CoopTask since we want to manage multiple tasks.
#include <CoopTaskBase.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>
#include <CoopMutex.h>
#include <BasicCoopTask.h>
#include <assert.h>
#include "Receiver.h"
#include "Arbitrate.h"
#include "OpticalFlow.h"
#include "SurfaceDistance.h"
#include "SimpleLog.cpp"
#include "Hover.h"
#include "constants.h"

#define NUMBER_OF_LAYER_TASKS 5
int taskToken = 1;

SLog logger(&Serial, 115200, LOGLEVEL);

CoopTask<void>* heartBeatTask = nullptr;
CoopTask<void>* movementCtrlTask = nullptr;
CoopTask<void>* hoverCtrlTask = nullptr;
CoopTask<void>* surfaceDistCtrlTask = nullptr;
CoopTask<void>* receiverCtrlTask = nullptr;
CoopTask<void>* arbitrateCtrlTask = nullptr;

/** 
This semaphore is used to avoid concurrent access to the receives data array during updateing this array
Receiver.cpp set and reset this semaphore. is set, other task can not read the internal data array via getData()
**/
CoopSemaphore taskSema(1, 1);

Receiver receiver(TASK_RECEIVER, &logger, &Serial2, 16, 17, true);

Hover hover(TASK_HOVER, &logger);
OpticalFlow flow(TASK_OPTICALFLOW, &logger, PIN_PMW3901);
SurfaceDistance distance(TASK_SURFACEDISTANCE, &logger);
Arbitrate arbitrate(TASK_ARBITRATE, &logger);


//Receiver *receiver;

// Task 1 : HeartBeat (1 second)
void HeartBeatFunction() {
//  taskSema.wait();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for(;;) {
    //logger.info("blink");
    yield();
    digitalWrite(LED_BUILTIN, LOW);
    delay(HB_BLINK_FREQ);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(HB_BLINK_FREQ);
    //CoopTask<void>::sleep();
    yield();
  }
}


// Task 2: use PMW3901 optical flow sensor
void MovementControlFunction() {
//  taskSema.wait();
  unsigned long lastMillis = millis();
  int16_t deltaX,deltaY;
  if (flow.begin(&receiver) == false) {
    logger.error("MAIN : can't start OpticalFlow");
    return;
  }
  for(;;) {
    //Serial.println("run MovementControlFunction");
    flow.update();
    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }

}


// Primary Task Hovering is the base functionality of the complete PODRacer System
// 
void HoverControlFunction() {
  unsigned long lastMillis = millis();
  if (hover.begin(&receiver) == false ) {
    logger.error("MAIN : can't start Hover object");
    return;
  }
  for(;;) {
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }

}


// 
void SurfaceDistanceControlFunction() {
  unsigned long lastMillis = millis();
  for(;;) {
    //flow.readMotionCount(&deltaX, &deltaY);
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
    logger.error("MAIN : can't start Receiver object");
    return;
  }
  for(;;) {
    /** read latest hardware-receiver data into SDATA struct **/
    receiver.update();
    if ((millis() - lastMillis) > LOOP_TIME) {
      lastMillis = millis();
      yield();
    }
    delay(LOOP_TIME);
  }

}
// Task: arbitrate 
void ArbitrateControlFunction() {
  unsigned long lastMillis = millis();
  if (&receiver) {
    if (!arbitrate.begin(&receiver)) {
      logger.error("MAIN : can't start arbitrate object");
    }
  }
  else {
    logger.error("arbitrate.begin() - no receiver object");
  }
  for(;;) {
    if (flow.isUpdated()) {
      ;
    }
    arbitrate.update();

    //flow.readMotionCount(&deltaX, &deltaY);
    if ((millis() - lastMillis) > LOOP_TIME) {
      yield();
    }
    delay(LOOP_TIME);
  }
  
}

bool DelayCoopFunction(int ms) {
  return true;
}

bool SleepCoopFunction() {
  Serial.println("run SleepCoopFunction");
  Serial.flush();
  delay(LOOP_TIME);
  if (heartBeatTask) heartBeatTask->wakeup();
  if (hoverCtrlTask) hoverCtrlTask->wakeup();
  if (movementCtrlTask) movementCtrlTask->wakeup();
  if (surfaceDistCtrlTask) surfaceDistCtrlTask->wakeup();
  if (receiverCtrlTask) receiverCtrlTask->wakeup();
  if (arbitrateCtrlTask) arbitrateCtrlTask->wakeup();
  #if defined (LOG_TASK_ALL)
    logger->debug ("SleepCoopFunction wakeup tasks done");
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
  logger.info("setup...");

  //-----------------------------------------
  logger.info("create tasks....");

  heartBeatTask = new CoopTask<void>(F("HeartBeat"),HeartBeatFunction);
  movementCtrlTask = new CoopTask<void>(F("MovementCtrl"), MovementControlFunction);
  hoverCtrlTask = new CoopTask<void>(F("HoverCtrl"), HoverControlFunction);
  surfaceDistCtrlTask= new CoopTask<void>(F("SurfaceDist"), SurfaceDistanceControlFunction);
  receiverCtrlTask = new CoopTask<void>(F("RECEIVER"),ReceiverControlFunction);
  arbitrateCtrlTask = new CoopTask<void>(F("ARBITRATE"),ArbitrateControlFunction);
  logger.info("all tasks ready");

  //-----------------------------------------
  logger.info("schedule tasks");

Serial.print("heartBeatTask:"); Serial.println((unsigned long)&heartBeatTask);
  runCoopTasks(nullptr, nullptr, SleepCoopFunction);


  heartBeatTask->scheduleTask();
  movementCtrlTask->scheduleTask();
  surfaceDistCtrlTask->scheduleTask();
  hoverCtrlTask->scheduleTask();
  receiverCtrlTask->scheduleTask();
  arbitrateCtrlTask->scheduleTask();
  logger.info("all tasks scheduled");
}



void loop() {
  // put your main code here, to run repeatedly:
  //runCoopTasks(nullptr, DelayCoopFunction, SleepCoopFunction);
  runCoopTasks(nullptr, nullptr, nullptr);
  //Serial.println("running tasks....");
  yield();
}
