#ifndef SFM_H
#define SFM_H
#include "../SFM_Common_C/SFM.h"


void* listen_thread_func(void* arg);
void* write_thread_func(void* arg);
void* interface_thread_func(void* arg);


extern _Atomic volatile sig_atomic_t interrupted;
extern _Atomic bool job_running;
extern struct queue* received_messages;
extern char read_write;

#endif //SMF_H
