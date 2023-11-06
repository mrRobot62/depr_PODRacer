# Implementierungs Hinweise

## 06.11.2023 BK
setError(), erweitert umd einen Parameter taskID & code.
Ein Fehler besteht nun aus zwei Teilen Bit0-Bit3 enthält den Code und Bit4-Bit7 die TaskID.
Somit können wir später in einer Auswertung deutlich einfacher Fehlercodes auswerten
Pro Tasks können 15 verschiedene ErrorCodes definiert werden
Insgesamt 16 Tasks können genutzt werden 

Beispiel: 
01010100  => Task 5 & Code 4
00010011  => Task 1, Code 3


## 05.11.2023 BK
Nachdem es mehrere Probleme mit Abstürzen gab, habe ich aktuell nur HOVERING/MIXER/RECEIVER aktiviert. Das primäre Problem mit Abstürzen lag an einem zu kleinen buffer innerhalb von SimpleLOG. Dieser intere Buffer habe ich auf 200 erhöht.

Durchstich gelungen ! Man kann nun sowohl in der Logausgabe als auch in Betaflight sehen, das sich der copter dreht/bewegt sobald man gearmt hat und mit dem Gimbal steuerbefehle sendet. Zum Versenden der Daten über SBUS muss zwingend wieder ein Mapping auf die 
channel_calibration Werte durchgeführt werden. Die Liste enthält
{MIN_SBUS, MAX_SBUS, GIMBAL_MIN, GIMBAL_MAX}
READ liest nun die SBUS signale und mapped diese auf GIMBAL_xxxxx.
WRITE macht es genau umgekehrt mapped von GIMBAL zurück auf SBUS
und dann funktioniert es auch mit BetaFlight :-) 

## 02.11.2023 BK
EmergencyTask implementiert (aber noch nicht getestet). Dieser Task wird in Main innerhalb der BlinkPatternFunction() aufgerufen um ein BlinkPattern zu setzen.

```
    if (emergency.isEmergencyStop()) {
      blink_pattern = PATTERN_EMERGENCY;
    } else if (receiver.isPreventArming() || emergency.isPreventArming()) {
      blink_pattern = PATTERN_PREVENTARMING;
    } else if (!receiver.isArmed() || emergency.isArmed()) {
      blink_pattern = PATTERN_DISARMED;
    }
```

## 01.11.2023 BK
MIXER gesplittet in _HoverMixer und _RPYMixer. In der update() Funktion von Mixer.cpp.
Zuerst wird _HoverMixer aufgerufen. Dieser Mixer nutzt die Daten aus HoverTask & SurfaceDistance
Anschließnd wird _RPYMixer aufgerufen.
Dieser Mixer wird entweder von FlowTask oder von SteeringTask genutzt.

Sind beide Sub-Mixer abgearbeitet werden die Daten in das zentralen Mixer-Struct kopiert und dem Receiver übergeben.

## 31.10.2023 BK
### Mixer leitet jetzt Hover Data zum Receiver durch
Wichtiger Schritt geschafft. Aus der Main-Methode, wird kann der Mixer nun den HoverTask "abfragen" und seine daten an den Receiver weiterleiten. Das ist ein wichtiger Schritt.

### Hovering-Task
setzt `_bbd.data.ch[HOVERING]` auf den POTI-Wert ist der Wert, ~~ändert sich der Wert zum vorherigen mal, dann wird
`_bbd.data.updated` auf `true` gesetzt~~ - HOVER setzt `_bbd.data.updated` immer auf true. Serial-Plotout zeigt nur geänderte Werte um ein Output-Overkill zu vermeiden

### Disarming - Blinking-Pattern
Blink-Pattern, wenn der PODRacer Disarmed ist (2x 250ms, 2x100ms wiederholden).

### PreventArming - Blinking-Pattern
Wenn _isPreventArming = true, dann wird ein schnell blinkendes Muster (1x100ms ON, 1x100ms OFF wiederholend) angezeigt. Das deutet dann darauf hin, das einer der Sticks oder Poti nicht korrekt sind und somit das Arming nicht möglich ist.

### PreventArming - BitMasking
Arming ist nur erlaubt wenn eine definierter Status vorhanden ist.
Es darf nur dann gearmed werden wenn:
* R/P/Y in CenterPosition
* Throttle & Thrust auf MIN-Position sind
* Wurde einmal erfolgreich gearmt und es wird disarmed dann muss anschließend wieder alles auf Center und Min stehen sonst ist kein arming möglich
  
> Vermutlich ist der Throttle-Poti oder der Thrust-Stick meistens nicht in der MIN-Position, das ist der Grund warum es 
so wichtig ist, das man nur armen kann, wenn es ungefährlich ist.

**Lösung:**
Verwendet wird eine armingMask. Hier werden nur die unteren 4Bits verwendet

* armingMask   0b0000 0000 (hier werden die Bits gesetzt)
* armingOKMask 0b0000 1010 (vergleich gegen dieses Muster)

**Kürzel**

- **ASO** = ArmingSwitchOFF
- **AS1** = ArmingSwitchON
- **SP1** = RPY=CP, T&H = MIN
- **SP0** = ein Stick/Poti ist nicht in CP oder MIN 

#### Beispiel 1 : Disarmed und Sticks in CP bzw. MIN, Ideal-Zustand
|Beispiel|Bit 3|Bit 2|Bit 1|Bit 0|Info|Maskierung|Arming|
|---|:-:|:-:|:-:|:-:|---|:-:|--:|
|-|0|0|0|0|intial|nein|nein|
|AS0+SP1|0|0|1|1|AS0 setzt B0, SP1 setzt B1|0011 & 1010 = 0011|nein|
|AS0+SP0|0|0|0|1|B0 bleibt, SP0+AS0 => B3=0 B1=0|0001 & 1010 =0001|nein|
|AS1+SP1|1|0|1|0|B3 durch AS1, SP1 setzt B2, AS1 reset B0|1010 & 1010=1010|ja|

#### Beispiel 2 : PODRacer war gearmed wird nun disarmed, Throttle nicht auf MIN 
|Beispiel|Bit 3|Bit 2|Bit 1|Bit 0|Info|Maskierung|Arming|
|---|:-:|:-:|:-:|:-:|---|:-:|--:|
|-|1|0|1|0|Arming aktiv|-|ja|
|AS0+SP0|0|0|0|1|wird disarmed, AS0 setzt B0|0001 & 1010 = 0001|nein|
|AS1+SP0|1|0|0|1|arming, AS1 setzt B3, SP0 reset B1, B0 unverändert|1001 & 1010 = 1001|nein|
|AS1+SP1|1|0|1|1|arming, AS1 setzt B3, SP1 set B1, B0 unverändert|1011 & 1010 = 1011|nein|
|AS0+SP1|0|0|1|0|disarming B3=0, reset B0, SP1 set B1|0010 & 1010 = 0010|nein|
|AS1+SP1|1|0|1|0|arming B3=1, SP1 set B1|0010 & 1010 = 0010|ja|

## 29.10.2023 BK

### Klasse Blackbox
Diese Klasse hängt an TaskAbstract dran und steht jedem Task zur Verfügung. Hierüber kann ein Datenstream auf die SDCard geschrieben werden. Ich mach das immer so, das ich bevor ich das in unserem Projekt einbaue mit Test-Sketches arbeite um eine Klasse so gut es geht zu testen. Dort funktioniert die Klasse Blackbox recht gut.

Im Test (Test-Sketch) läuft das, beim Aufruf wird immer eine neue Datei mit einer Sequenz-Nummer generiert. In diese wird auch reingeschrieben.

In unserer FW ist das Ganze zwar inkludiert aber ich muss das nach mit dem ARMING-SWITCH korrekt verbinden. Loggen macht ja nur Sinn, wenn auch gearmt ist.

> **Wichtig:** die SDCard wird wie der PMW3901 über SPI angeschlossen. Jedes SPI-Device benötigt eine eigene CS(Chip-Select PIN). Aktuell GPIO5 = PMW3901, GPIO32=SDCard (siehe constants.h)

### Neue typedef struct BBD
```
typedef struct {
  uint16_t header;
  long millis;                          // milliseconds
  // data
  uint8_t task_id;                      // produces by task-id
  bool updated;                         // set by tasks
  bool failsafe;                        // set by receiver
  bool lost_frame;                      // set by receiver
  bool armingState;                     // true=armed, false=disarmed
  uint8_t groupA;                       // can be used to group data 1. order
  uint8_t groupB;                       // can be used to group data 2. order
  uint16_t ch[NUMBER_CHANNELS];         // channel data (0=ch1, 1=ch2, 2=ch3, ...)
  //uint16_t ch_w[NUMBER_CHANNELS];       // channel data (0=ch1, 1=ch2, 2=ch3, ...) WRITE 
  long ldata[DATA_SIZE];          // an be used for long values  
  double fdata[DATA_SIZE];        // can be used for float values
  double pid_rpyth[DATA_SIZE];    //(R/P/Y/T/H);
  uint16_t crc;
} BlackBoxStruct;

typedef union {
  BlackBoxStruct data;
  byte bytes [sizeof(BlackBoxStruct)];
} BBD;

```

Diese Struktur stellt TaskAbstract grundlegend zur Verfügung für jeden abgeleiteten Task. Jeder Task nutzt nun diese Struktur und kann seine Informationen einfügen. Diese Informationen laufen über den Mixer an den Receiver und von dort zum FlightController.

Zusätzlich ist diese Struktur nun so aufgebaut, das auch die Klasse Blackbox damit arbeiten kann und exakt diese Struktur auf SDKarte kopiert

Beispiel für ein Dump dieser Struktur:
```
-- dump-start (216 bytes) -------------------------
EF FE 00 00 B4 01 00 00 0A 0B F0 C0 B0 A0 AA 05 
AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 
AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 AA 05 00 00 
0C FE FF FF 0C FE FF FF 0C FE FF FF 0C FE FF FF 
0C FE FF FF 0C FE FF FF 0C FE FF FF 0C FE FF FF 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
6F 12 83 C0 CA 21 09 40 6F 12 83 C0 CA 21 09 40 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
00 00 00 00 00 00 F0 3F 00 00 00 00 00 00 F0 3F 
FF 00 97 2C 00 00 00 00 
-- dump-end ------------------------------------

```
WICHIG: der Byte-Dump ist als Little-Endian gespeichert. Das heißt eine uint16_t value (z.B. 0xFEEF) wird im Byte-Stream als EF FE gespeichert.

#### BBD Detail-Informationen

* `uint16_t header`
	* ist immer 0xFEEF und dient als "Satz-Anfangs-kennzeichen"
* `bool updated`
	* **muss** von jedem Task gesetzt werden, wenn es updates in der ch-Liste setzt. Sonst ignoriert der Mixer das
* `bool armingState`
	* **sehr wichtig** für Tasks nur wenn der State TRUE ist, darf der Task Daten anpassen und updated auf TRUE setzen !
* `uint8_t groupA & groupB`
	* kann verwendet werden, wenn man seine Daten strukturieren möchte. Kann später ggf. über den Log-Analyzer gefiltert werden
* `double pid_rpyth[DATA_SIZE]`
	* hier können TASKs ihre PID Werte reinschreiben insgesamt stehen 8 freie Plätze zur Verfügung. In der Regel benötigt man 3-4 (ROLL;PITCH;YAW;?)
* `uint16_t ch[NUMBER_CHANNELS]`
	* hier schreiben die TASK ihre geänderten (in der Regel relative) Abweichungen zur CenterPoint (1500) rein. Ausnahme HOVER dieser Task schreibt absolute Werte. Aktuell nutzen wir nur CH1-CH8
* `long ldata[DATA_SIZE]`
	* TASK spezifische Werte (falls notwendig), die auch geloggt werden können. 8 unterschiedliche Long-Werte können pro Task gespeichert werden
* `double fdata[DATA_SIZE]`
	* dito zu ldata allerdings für Dezimal-Zahlen (3.1415)
* `bool fail_safe & lost_frames`
	* wird durch Receiver gesetzt

### Arming
Es ist wichtig, das das Ganze nur dann läuft, wenn der PODRacer auch „scharf“ geschaltet ist (ARMING) - mit dem roten Schalter. Wenn OFF, passiert nix, wenn ON, dann drehen die Rotoren etc.

Wichtig beim ARMEN ist das ein Check gemacht wird, das die Gimbals im Center sind, Throttel auf MIN und THRUST auch auf MIN stehen, erst dann darf gearmt werden ansonsten könnte es passieren, das der PODRacer direkt los brettert sobald man den Arming-Switch setzt :-O

Ich habe damit exemplarisch begonnen aber das klappt noch nicht perfekt. Hört sich einfach an aber in einem Task-Umfeld wo jeder Task unabhängig läuft ist das etwas tricky.

### Logging:
Beim Logging kann man jetzt auch den TASK-Namen mit angeben, dann wird im Log auch angezeigt von wem der Logeintrag kommt. Alle tasks haben im Constructor eine Variable _tname = „xxxxx“ stehen, der kann jetzt bei logger->info(„text“,_tname) als zweiter Parameter mit gegeben werden. Macht man das nicht erscheint ein „?“ in der Logausgabe zu diesem Logeintrag.
Commit
Ich habe Deine Änderungen gemerged aber ich befürchte da ist mir ein Fehler unterlaufen

`logger->info(buffer, _tname);`
Hier ein Bespiel es wird der Buffer ausgegeben und `_tname` im Log entsprechend ausgegeben
`10642| INFO|HOVER|H:1000` (hier HOVER)

`10642| INFO|    ?|H:1000` (wenn man _tname vergessen hat)

