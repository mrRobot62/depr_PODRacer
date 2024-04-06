#ifndef _GLOBAL_VARS_H_
#define _GLOBAL_VARS_H_

#include <CoopTaskBase.h>
#include <CoopTask.h>

#define TASK_LIST_SIZE 10

typedef struct {
  CoopTask<void> *list[TASK_LIST_SIZE];
} TaskList;

TaskList taskList ;

#endif