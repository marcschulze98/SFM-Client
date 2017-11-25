#define _POSIX_C_SOURCE 200809L
#define DEFAULT_BUFFER_LENGTH 128
#define DEFAULT_NAME_LENGTH 32
#define POLL_TIMEOUT 200
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

#include <sys/socket.h>
#include <stdint.h>
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
#include <math.h>

struct string
{
	char* data;
	uint_fast32_t length;
};

struct linked_list
{
	pthread_mutex_t mutex;
	void* data;
	struct linked_list* next;
};

void realloc_write(struct string* target, char c, uint_fast32_t offset);
void reset_string_size(struct string* stringbuffer, uint_fast32_t buffer_size);
void printBits(size_t size, void* ptr);
void adjust_string_size(struct string* target ,uint_fast32_t size);
void swap_endianess_16(uint16_t * byte);
void free_linked_list_pointer(struct linked_list* currentitem);
bool realloc_read(struct string* target, unsigned short bytes_to_read, int socket_fd, uint_fast32_t offset);
bool get_message(struct string* message, int socket_fd);
char* convert_string(char* source);
void send_string(char* message, int socket_fd);
