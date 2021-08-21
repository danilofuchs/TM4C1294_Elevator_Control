#include "main_thread.h"

#include "kernel_info.h"

void mainThread(void* arg) {
  main_thread_t* this = (main_thread_t*)arg;

  osMutexAcquire(this->args.uart_write_mutex, osWaitForever);
  printKernelState();
  printThreadInfo();
  osMutexRelease(this->args.uart_write_mutex);

  while (1)
    ;
}