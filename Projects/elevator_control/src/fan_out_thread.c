#include "fan_out_thread.h"

#include "signal.h"

static void putMessage(osMessageQueueId_t queue, signal_t* signal) {
  osMessageQueuePut(queue, signal, NULL, osWaitForever);
}

static void fanOut(signal_t* signal, fan_out_thread_args_t* args) {
  if (signal->code == signal_unknown) return;

  if (signal->code == signal_system_initialized) {
    // Initialization affects all elevators
    putMessage(args->left_elevator_signal_queue, signal);
    putMessage(args->center_elevator_signal_queue, signal);
    putMessage(args->right_elevator_signal_queue, signal);
    putMessage(args->height_checker_signal_queue, signal);
    return;
  }

  if (signal->code == signal_height_changed) {
    // Height change is handled by the height checker thread
    putMessage(args->height_checker_signal_queue, signal);
    return;
  }

  switch (signal->elevator_code) {
    case elevator_code_left:
      putMessage(args->left_elevator_signal_queue, signal);
      return;
    case elevator_code_center:
      putMessage(args->center_elevator_signal_queue, signal);
      return;
    case elevator_code_right:
      putMessage(args->right_elevator_signal_queue, signal);
      return;
  }
}

void fanOutThread(void* arg) {
  fan_out_thread_t* this = (fan_out_thread_t*)arg;

  signal_t signal;
  while (1) {
    osStatus_t status = osMessageQueueGet(this->args.incoming_signal_queue,
                                          &signal, NULL, osWaitForever);

    if (status != osOK) continue;

    fanOut(&signal, &this->args);

    // signalDebug(&signal);
  }
}