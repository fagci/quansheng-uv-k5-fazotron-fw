#pragma once

#include "./driver/uart.hpp"
#include "driver/system.hpp"
#include <stddef.h>
#include <stdint.h>

class Scheduler {
public:
  static constexpr uint8_t TASKS_MAX = 64;

  typedef struct {
    const char *name;
    void (*handler)();
    uint16_t interval;
    uint16_t countdown;
    bool continuous;
    uint8_t priority;
    bool active;
  } Task;

  static Task *taskAdd(const char *name, void (*handler)(), uint16_t interval,
                       bool continuous, uint8_t priority) {
    UART_logf(3, "TaskAdd(%s)", name);
    if (tasksCount == TASKS_MAX) {
      return NULL;
    }

    uint8_t insertI;
    for (insertI = 0; insertI < tasksCount; ++insertI) {
      if (tasks[insertI].priority > priority) {
        break;
      }
    }

    if (insertI < tasksCount) {
      for (uint8_t i = tasksCount; i > insertI; --i) {
        tasks[i] = tasks[i - 1];
      }
    }

    tasks[insertI] = {name,       handler,  interval, interval,
                      continuous, priority, false};
    tasksCount++;
    return &tasks[insertI];
  }

  static void taskRemove(void (*handler)()) {
    uint8_t i;
    Task *t;
    for (i = 0; i < tasksCount; ++i) {
      t = &tasks[i];
      if (t->handler == handler) {
        t->handler = NULL;
        UART_logf(3, "TaskRemove(%s)", t->name);
        tasksCount--;
        break;
      }
    }
    for (; i < tasksCount; ++i) {
      if (tasks[i].handler == NULL && tasks[i + 1].handler != NULL) {
        tasks[i] = tasks[i + 1];
        tasks[i + 1].handler = NULL;
      }
    }
  }

  static bool taskExists(void (*handler)()) {
    uint8_t i;
    Task *t;
    for (i = 0; i < tasksCount; ++i) {
      t = &tasks[i];
      if (t->handler == handler) {
        return true;
      }
    }
    return false;
  }

  static void taskTouch(void (*handler)()) {
    Task *t;
    for (uint8_t i = 0; i < tasksCount; ++i) {
      t = &tasks[i];
      if (t->handler == handler) {
        t->countdown = 0;
        UART_logf(3, "TaskTouch(%s)", t->name);
        return;
      }
    }
  }

  static void tasksUpdate() {
    Task *task;
    if (Now() - lastUpdate == 0) {
      return;
    }
    lastUpdate = Now();
    for (uint8_t i = 0; i < tasksCount; ++i) {
      task = &tasks[i];
      if (task->active && task->handler && task->countdown) {
        --task->countdown;
      }
    }

    for (uint8_t i = 0; i < tasksCount; ++i) {
      tasks[i].active = true;
    }
    for (uint8_t i = 0; i < tasksCount; ++i) {
      task = &tasks[i];
      if (task->handler && !task->countdown) {
        handle(task);
        if (task->continuous) {
          task->countdown = task->interval;
        } else {
          taskRemove(task->handler);
        }
      }
    }
  }

private:
  static uint32_t lastUpdate;
  static uint8_t tasksCount;
  static Task tasks[TASKS_MAX];

  static void handle(Task *task) {
    UART_logf(3, "%s::handle() start", task->name);
    task->handler();
    UART_logf(3, "%s::handle() end", task->name);
  }
};
