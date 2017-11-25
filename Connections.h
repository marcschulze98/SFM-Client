
#define _POSIX_C_SOURCE 200809L
#define DEFAULT_BUFFER_LENGTH 256
#define DEFAULT_NAME_LENGTH 32

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <poll.h>
#include <stdatomic.h>

extern _Atomic volatile sig_atomic_t interrupted;

struct string
{
	char* data;
	unsigned long length;
};

void* listen_thread_func(void* restrict arg);
void* write_thread_func(void* restrict arg);
void* interface_thread_func(void* restrict arg);
inline void realloc_write(struct string* target, char c, size_t current_char);
inline void reset_string_size(struct string* stringbuffer, size_t buffer_size);
inline char* convert_string(char* source);


