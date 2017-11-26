#include "SFM_Client.h"

static void print_welcome_message(void);
static void print_help(void);
static void handle_interface_command(char* command);
static void get_input(void);

_Atomic bool job_running = false;
struct queue* received_messages;

pthread_t write_thread;
pthread_t listen_thread;
int* socket_fd_pointer;

void* interface_thread_func(void* arg)
{
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGINT);
    pthread_sigmask (SIG_BLOCK, &signal_mask, NULL); //block SIGINT

	socket_fd_pointer = arg;

	print_welcome_message();
	if((read_write == '1' || read_write == '3')) //get messages at beginning (probably motd too)
	{
		pthread_create(&listen_thread, NULL, listen_thread_func, socket_fd_pointer);
		pthread_join(listen_thread, NULL);
	}

	get_input();

	pthread_exit(0);
}

static void get_input()
{
	struct string user_input = new_string(DEFAULT_BUFFER_LENGTH);
	struct string bye = { .data = "\x00\x05/bye\0", .length = 7, .capacity = 7};
	struct pollfd stdin_ready = { .fd = fileno(stdin), .events = POLLIN, .revents = 0};
	bool stdin_has_data_left = false;
	int current_char;

	while(!interrupted)
	{
		printf("Befehl eingeben: ");
		fflush(stdout);
		while(true) 									//poll stdin and check for interrupt, if stdin has data, read until newline then break
		{
			if(stdin_has_data_left || poll(&stdin_ready, 1, POLL_TIMEOUT))
			{
				stdin_has_data_left = true;
				if((current_char = getchar()) == '\n' || current_char == EOF)
				{
					stdin_has_data_left = false;
					break;
				}
				realloc_write(&user_input, (char)current_char, user_input.length);
				user_input.length++;

			} else if(interrupted) {
				break;
			}
		}
		if(interrupted)
			break;
		user_input.data[user_input.length] = '\0';
		handle_interface_command(user_input.data);
		reset_string(&user_input, DEFAULT_BUFFER_LENGTH);
	}

	free(user_input.data);
	free(received_messages);

	send_string(&bye, *socket_fd_pointer);

	pthread_exit(0);
}

static void print_welcome_message(void)		//welcome messages
{
	printf("\n\"help\" eingeben um verfügbare Befehle auszugeben\n\n");
	fflush(stdout);
}
static void print_help(void)		//availabe commands etc.
{
	printf("\n\"?\" oder \"help\": Diese Hilfenachricht anzeigen\n");
	printf("\"send\": Nachricht verfassen und senden\n");
	printf("\"get\": Nachrichten abholen\n");
	printf("\"logout\": Vom Server ausloggen und Client beenden\n\n");

	fflush(stdout);
}

static void handle_interface_command(char* command) //do action according to input
{
	if(strncmp(command, "help", strlen("help")) == 0)
	{
		print_help();
	} else if(strncmp(command, "?", strlen("?")) == 0) {
		print_help();
	} else if(strncmp(command, "logout", strlen("logout")) == 0) {
		interrupted = 1;
	} else if((strncmp(command, "send", strlen("send")) == 0) && (read_write == '1' || read_write == '2')) {
		pthread_create(&write_thread, NULL, write_thread_func, socket_fd_pointer);
		pthread_join(write_thread, NULL);
	} else if((strncmp(command, "get", strlen("get")) == 0) && (read_write == '1' || read_write == '3')) {
		pthread_create(&listen_thread, NULL, listen_thread_func, socket_fd_pointer);
		pthread_join(listen_thread, NULL);
	}


	else {
		printf("Befehl \"%s\" unbekannt oder nicht verfügbar\n", command);
	}

	fflush(stdout);

}
