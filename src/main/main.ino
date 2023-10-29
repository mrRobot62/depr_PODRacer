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

#define NUMBER_OF_LAYER_TASKS 5
char * _tname = "MAIN";
int taskToken = 1;

SLog logger(&Serial, 115200, LOGLEVEL);

Blackbox bb(&logger, BLACKBOX_CS_PIN);

CoopTask<void>* blinkpatternTask = nullptr;
CoopTask<void>* movementCtrlTask = nullptr;
CoopTask<void>* hoverCtrlTask = nullptr;
CoopTask<void>* steeringCtrlTask = nullptr;
CoopTask<void>* surfaceDistCtrlTask = nullptr;
CoopTask<void>* receiverCtrlTask = nullptr;
CoopTask<void>* mixerCtrlTask = nullptr;
CoopTask<void>* blackboxCtrlTask = nullptr;

/** 
This semaphore is used to avoid concurrent access to the receives data array during updateing this array
Receiver.cpp set and reset this semaphore. is set, other task can not read the internal data array via getData()
**/
CoopSemaphore taskSema(1, 1);
HardwareSerial lidarSerial(1);


BlinkPattern blinkP(TASK_HB, &logger);
Hover hover(TASK_HOVER, &logger,&bb);
SurfaceDistance distance(TASK_SURFACEDISTANCE, &logger, &lidarSerial, &bb);
OpticalFlow flow(TASK_OPTICALFLOW, &logger, PIN_PMW3901, &bb);
Steering steering(TASK_STEERING, &logger, &bb);
Receiver receiver(TASK_RECEIVER, &logger, &Serial2, 16, 17, true, &bb);
Mixer mixer(TASK_MIXER, &logger, &bb);
//Blackbox bb(TASK_MIXER, &logger, CS_PIN);

//Receiver *receiver;
uint8_t blink_pattern = 0;

// Task: HeartBeat (1 second)
void BlinkPatternFunction() {
//  taskSema.wait();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  blinkP.begin(&blink_pattern);
  char buffer[100];
  for(;;) {
    blink_pattern = hover.getError() | flow.getError() | steering.getError() | receiver.getError();
    //logger.printBinary("BlinkPatternFunction-Pattern:", blink_pattern);
    //blinkP.update(blink_pattern);
    blinkP.update(0);
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
    if (!hover.hasError()) {
      hover.update();
      if ((millis() - lastMillis) > LOOP_TIME) {
        yield();
      }
      delay(LOOP_TIME);
    }
  }

}

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
  }
}


// Task: control SBUS activites (Receiver -> EPS32 -> FlightController)
void ReceiverControlFunction() {
  unsigned long lastMillis = millis();
  if (!receiver.begin()) {
    logger.error("MAIN : can't start Receiver object", _tname);
    return;
  }
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
      logger.error("Receiver has Error", _tname);
    }
  }
}
// Task: Mixer 
void MixerControlFunction() {

  unsigned long lastMillis = millis();
  if (&receiver) {
    if (!mixer.begin(&receiver)) {
      logger.error("MAIN : can't start mixer object", _tname);
    }
  }
  else {
    logger.error("mixer.begin() - no receiver object", _tname);
  }

  for(;;) {
    // hovering is the base function of the podrace
    if (hover.isUpdated()) {
      mixer.update(&hover);
    }
    // SurfaceDistance is the next higher level. The sensor set the height for the podrace
    if (distance.isUpdated()) {
      mixer.update(&distance);
    }  
    // 
    if (steering.isUpdated()) {
      mixer.update(&steering);
    } 
    // 
    if (flow.isUpdated()) {
      mixer.update(&flow);
    }  


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
  if (blinkpatternTask) blinkpatternTask->wakeup();
  if (hoverCtrlTask) hoverCtrlTask->wakeup();
  if (steeringCtrlTask) steeringCtrlTask->wakeup();
  if (movementCtrlTask) movementCtrlTask->wakeup();
  if (surfaceDistCtrlTask) surfaceDistCtrlTask->wakeup();
  if (receiverCtrlTask) receiverCtrlTask->wakeup();
  if (mixerCtrlTask) mixerCtrlTask->wakeup();
  if (blackboxCtrlTask) blackboxCtrlTask->wakeup();
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

  //-----------------------------------------
  logger.info("create tasks....", _tname);

  blinkpatternTask = new CoopTask<void>(F("BLINK"),BlinkPatternFunction);
  steeringCtrlTask = new CoopTask<void>(F("STEER"),SteeringControlFunction);
  movementCtrlTask = new CoopTask<void>(F("OFLOW"), MovementControlFunction);
  hoverCtrlTask = new CoopTask<void>(F("HOVER"), HoverControlFunction);
  surfaceDistCtrlTask= new CoopTask<void>(F("SURFD"), SurfaceDistanceControlFunction);
  receiverCtrlTask = new CoopTask<void>(F("RECV"),ReceiverControlFunction);
  mixerCtrlTask = new CoopTask<void>(F("MIXER"),MixerControlFunction);
  logger.info("all tasks ready", _tname);

  //-----------------------------------------
  logger.info("schedule tasks", _tname);
  runCoopTasks(nullptr, nullptr, SleepCoopFunction);

  blinkpatternTask->scheduleTask();
  movementCtrlTask->scheduleTask();
  surfaceDistCtrlTask->scheduleTask();
  hoverCtrlTask->scheduleTask();
  steeringCtrlTask->scheduleTask();
  receiverCtrlTask->scheduleTask();
  mixerCtrlTask->scheduleTask();
  logger.info("all tasks scheduled", _tname);

  //<todo> should be moved if armed=true
  bb.begin();
}



void loop() {
  // put your main code here, to run repeatedly:
  //runCoopTasks(nullptr, DelayCoopFunction, SleepCoopFunction);
  runCoopTasks(nullptr, nullptr, nullptr);
  //Serial.println("running tasks....");
  yield();
}
