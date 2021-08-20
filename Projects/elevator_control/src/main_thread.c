#include "main_thread.h"

#include "kernel_info.h"

void mainThread(void* arg) {
  main_thread_t* this = (main_thread_t*)arg;

  osMutexAcquire(this->args.uart_write_mutex_id, osWaitForever);
  printKernelState();
  printThreadInfo();
  osMutexRelease(this->args.uart_write_mutex_id);

  *this->args.left_elevator = (elevator_t){
      .code = elevator_code_left, .direction = elevator_direction_unknown};
  *this->args.center_elevator = (elevator_t){
      .code = elevator_code_center, .direction = elevator_direction_unknown};
  *this->args.right_elevator = (elevator_t){
      .code = elevator_code_right, .direction = elevator_direction_unknown};

  while (1)
    ;
}