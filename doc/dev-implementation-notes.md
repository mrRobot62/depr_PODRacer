# Implementierungs Hinweise


## 30.10.2023 BK

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

