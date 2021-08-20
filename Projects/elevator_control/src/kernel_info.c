#include "kernel_info.h"

#include <stdbool.h>
#include <stdint.h>

#include "cmsis_os2.h"

// Utils
#include "utils/uartstdio.h"

void printKernelInfo(void) {
  osVersion_t osv;
  char infobuf[16];
  if (osKernelGetInfo(&osv, infobuf, sizeof(infobuf)) == osOK) {
    UARTprintf("Kernel Information: %s\n", infobuf);
    UARTprintf("Kernel Version    : %d\n", osv.kernel);
    UARTprintf("Kernel API Version: %d\n\n", osv.api);
    UARTFlushTx(false);
  }
}

void printKernelState(void) {
  UARTprintf("Kernel State: ");
  switch (osKernelGetState()) {
    case osKernelInactive:
      UARTprintf("Inactive\n\n");
      break;
    case osKernelReady:
      UARTprintf("Ready\n\n");
      break;
    case osKernelRunning:
      UARTprintf("Running\n\n");
      break;
    case osKernelLocked:
      UARTprintf("Locked\n\n");
      break;
    case osKernelError:
      UARTprintf("Error\n\n");
      break;
  }
  UARTFlushTx(false);
}

void printThreadState(osThreadId_t thread_id) {
  osThreadState_t state;
  state = osThreadGetState(thread_id);
  switch (state) {
    case osThreadInactive:
      UARTprintf("Inactive\n");
      break;
    case osThreadReady:
      UARTprintf("Ready\n");
      break;
    case osThreadRunning:
      UARTprintf("Running\n");
      break;
    case osThreadBlocked:
      UARTprintf("Blocked\n");
      break;
    case osThreadTerminated:
      UARTprintf("Terminated\n");
      break;
    case osThreadError:
      UARTprintf("Error\n");
      break;
  }
}

void printThreadInfo(void) {
  osThreadId_t threads[SYSTEM_INFO_THREAD_COUNT_MAX];
  uint32_t number = osThreadEnumerate(threads, sizeof(threads));

  UARTprintf("Number of active threads: %d\n", number);
  for (int n = 0; n < number; n++) {
    const char* thread_name = osThreadGetName(threads[n]);
    UARTprintf("  %s (priority %d) - ", thread_name == NULL ? "?" : thread_name,
               osThreadGetPriority(threads[n]));
    printThreadState(threads[n]);
  }
  UARTprintf("\n");
  UARTFlushTx(false);
}
