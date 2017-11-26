#include "SFM_Client.h"

static void send_message(int* socket_fd_pointer);
void read_input(struct string* input, struct pollfd* stdin_ready, char insert_after);

void* write_thread_func(void* arg)
{
	sigset_t   signal_mask;
	sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGINT);
    pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	int* socket_fd_pointer = (int*)arg;

	send_message(socket_fd_pointer);

	pthread_exit(0);
}

void send_message(int* socket_fd_pointer)		//read input into different buffers, concatenate them afterwards TODO: maybe rewrite all into one buffer?
{

	struct string servername = { .data = malloc(DEFAULT_NAME_LENGTH), .length = 0, .capacity = DEFAULT_NAME_LENGTH};
	struct string username = { .data = malloc(DEFAULT_NAME_LENGTH), .length = 0, .capacity = DEFAULT_NAME_LENGTH};
	struct string message = { .data = malloc(DEFAULT_BUFFER_LENGTH), .length = 0, .capacity = DEFAULT_BUFFER_LENGTH};
	struct pollfd stdin_ready = { .fd = fileno(stdin), .events = POLLIN, .revents = 0};
	struct return_info return_codes;

	printf("Zielserver eingeben: ");
	fflush(stdout);
	read_input(&servername, &stdin_ready, '@');
	if(interrupted)
		goto cleanup;
	printf("Zieluser eingeben: ");
	fflush(stdout);
	read_input(&username, &stdin_ready, ':');
	if(interrupted)
		goto cleanup;
	printf("Nachricht eingeben:\n");
	fflush(stdout);
	read_input(&message, &stdin_ready, '\0');
	if(interrupted)
		goto cleanup;


	uint32_t complete_length = servername.length + username.length + message.length;

	struct string send = { .data = malloc(complete_length+3), .length = 0, .capacity = complete_length};
	memcpy(send.data + send.length, servername.data, servername.length);
	send.length += servername.length;
	memcpy(send.data + send.length, username.data, username.length);
	send.length += username.length;
	memcpy(send.data + send.length, message.data, message.length);
	send.length += message.length;

	convert_string(&send);

	return_codes = send_string(&send, *socket_fd_pointer);
	if(return_codes.error_occured)
	{
		printf("Fehler beim senden der Nachricht aufgetreten: %s\n", strerror(return_codes.error_code));
	}

	free(send.data);

	cleanup:
	free(servername.data);
	free(username.data);
	free(message.data);
}

void read_input(struct string* input, struct pollfd* stdin_ready, char insert_after) //read input into the buffer, check for interrupts for early return
{
	int current_char;
	bool stdin_has_data_left = false;

	while(true)
	{
		if(stdin_has_data_left || poll(stdin_ready, 1, POLL_TIMEOUT))
		{
			stdin_has_data_left = true;
			if((current_char = getchar()) == '\n' || current_char == EOF)
			{
				break;
			}
			realloc_write(input, (char)current_char, input->length);
			input->length++;

		} else if(interrupted) {
			return;
		}
	}

	realloc_write(input, insert_after, input->length);
	input->length++;

}

