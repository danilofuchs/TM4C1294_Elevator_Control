#ifndef __SYSTEM_INFO_H_
#define __SYSTEM_INFO_H_

// Defines the maximum amount of threads to log
#define SYSTEM_INFO_THREAD_COUNT_MAX 8

void printKernelInfo(void);
void printKernelState(void);
void printThreadInfo(void);

#endif