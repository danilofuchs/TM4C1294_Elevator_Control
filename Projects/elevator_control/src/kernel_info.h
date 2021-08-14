#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

// Defines the maximum amount of threads to log
#define SYSTEM_INFO_THREAD_COUNT_MAX 8

void printKernelInfo(void);
void printKernelState(void);
void printThreadInfo(void);

#endif