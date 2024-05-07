#ifndef _GLOBAL_UTILS_H_
#define _GLOBAL_UTILS_H_

#define pragma once
#include <stddef.h>
#include <stdio.h>
#include <limits.h>

/** check if value is >= min and value <= max **/
inline bool isInRange(double value, double min, double max) {
  if ((value >= min) && (value <= max)) {
    return true;
  }
  return false;
}

/** check if value >= compareValue-offset and value <= compareValue+offset **/
inline bool isInInterval(int16_t value, int16_t compareValue, int16_t offset) {
  if ((value >= (compareValue-offset)) && (value <= (compareValue+offset))) {
    return true;
  }
  return false;
}

/** check if value is >= value-offset and value <= value+offset **/
inline bool isInInterval(int16_t value, int16_t offset) {
  if ((value >= (value-offset)) && (value <= (value+offset))) {
    return true;
  }
  return false;
}

/** check if a value (value) is in a range between compareValue-min and compareValue+max to due to some small noise from receiver values **/
inline bool isInInterval(int16_t value, int16_t compareValue, int16_t min, int16_t max) {
  if ((value >= (compareValue+min)) && (value <= (compareValue+max))) {
    return true;
  }
  return false;
}

inline int16_t centeredValue(int16_t value, int16_t centerValue, int8_t offset) {
  if ((value >= (centerValue - offset)) && (value <= (centerValue+offset))) {
    return centerValue;
  }
  return value;
}



#endif
