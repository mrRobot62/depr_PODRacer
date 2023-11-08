# PODRacer Error codes
Error are stored in an uint8_t variable. To store for all tasks different error codes we use bit masks

## Bit-Mask
7 6 5 4 3 2 1

x x x x y y y

xxxx = TaskID 1-15
yyyy = Code 1-15

To set an error call inside a task `setError(getID(), 0x??);` 

## Codecs

|Task|TaskID|ErrorCode|Code|Info|
|---|:-:|:-:|:-:|---|
|TASK_HOVER|1|00010001|1|Receiver object missing|
|TASK_OPTICALFLOW|2|00100001|1|Receiver object missing|
|TASK_OPTICALFLOW|2|00100010|2|PMW3901 object missing|
|TASK_OPTICALFLOW|2|00100011|3|PID controller not initialized|
|TASK_OPTICALFLOW|2|00100100|4|flow->begin() failed|
||||||
||||||
||||||
||||||
