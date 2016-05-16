#include "MicroBitFiber.h"

#include <stdio.h>

unsigned long ticks = 0;
Fiber mainFiber;
Fiber* currentFiber = &mainFiber;

int invoke(void (*entry_fn)(void *), void *param) {
  fprintf(stderr, "Unhandled: invoke()\n");
}

void schedule() {
  fprintf(stderr, "Unhandled: schedule()\n");
}

int scheduler_runqueue_empty() {
  fprintf(stderr, "Unhandled: scheduler_runqueue_empty()\n");
}
