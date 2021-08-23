#include "height_checker_thread.h"

#include "command.h"
#include "elevator.h"
#include "signal.h"

#define FLAG 0x01
#define FREQUENCY 500
#define TIMEOUT 200

static void sendCommand(command_t* command, osMutexId_t mutex) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(mutex, osWaitForever);
  UARTprintf(string);
  UARTFlushTx(false);
  osMutexRelease(mutex);
}

static void sendQueryCommand(elevator_code_t elevator_code, osMutexId_t mutex) {
  command_t command = {
      .code = command_query_height,
      .elevator_code = elevator_code,
  };

  sendCommand(&command, mutex);
}

typedef struct {
  osThreadId_t* thread;
} timer_callback_args_t;

static void timerCallback(void* arg) {
  timer_callback_args_t* args = (timer_callback_args_t*)arg;

  osThreadFlagsSet(*args->thread, FLAG);
}

// This thread fires events every second to query the height of the elevator.
// It is responsible for sending the query command to the elevator and
// waiting for a response from the fan_out_thread
void heightCheckerThread(void* arg) {
  height_checker_thread_t* this = (height_checker_thread_t*)arg;

  this->timer = osTimerNew(timerCallback, osTimerPeriodic,
                           &(timer_callback_args_t){
                               .thread = &this->thread_id,
                           },
                           &(osTimerAttr_t){.name = "Height Checker Timer"});
  osTimerStart(this->timer, FREQUENCY);

  signal_t signal;
  bool initialized = false;

  do {
    osMessageQueueGet(this->args.queue, &signal, NULL, osWaitForever);
    initialized = signal.code == signal_system_initialized;
  } while (!initialized);

  while (1) {
    osThreadFlagsWait(FLAG, osFlagsWaitAny, osWaitForever);
    osThreadFlagsClear(FLAG);

    sendQueryCommand(elevator_code_left, this->args.uart_write_mutex);
    osMessageQueueGet(this->args.queue, &signal, NULL, TIMEOUT);
    osMessageQueuePut(this->args.left_elevator_queue, &signal, NULL, 0);
  }
}