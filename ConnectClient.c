#include "SFM_Client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <locale.h>
#include <limits.h>
#include <netdb.h>

static void sigint_handler(int signum);
static struct string* read_input(void);
static int* init_connection(struct string* home_server);

_Atomic volatile sig_atomic_t interrupted = 0;
char read_write = 1;

int main (int argc, char **argv)
{

	struct string* username;
	struct string* password;
	struct string* home_server;
	struct string answer = new_string(2);
	pthread_t interface_thread;
	int* socket_fd_pointer;

	signal(SIGPIPE, SIG_IGN);


	if(!strstr(setlocale(LC_CTYPE, NULL),"UTF")) //try to set input locale to utf-8 or ascii
	{
		if(!setlocale(LC_CTYPE, "C"))
		{
			printf("Konnte die Locale nicht auf ein kompatibles Format setzen\nFormat der Nachrichten wird wahrscheinlich falsch sein\n");
		}
	}

	printf("Geben sie 1 zum senden und empfangen, 2 um nur zu senden und 3 um nur zu empfangen ein (letzte Zahl zählt, standard 1): "); //choose if Client listens, writes or both
	fflush(stdout);


	int c;
	while((c = getchar()) != '\n' && c != EOF)
	{
		if(c == '1')
			read_write = (char)c;
		if(c == '2')
			read_write = (char)c;
		if(c == '3')
			read_write = (char)c;
	}

	printf("Addresse des Homeserver eingeben: ");
	fflush(stdout);
	home_server = read_input();

	socket_fd_pointer = init_connection(home_server);


	printf("Usernamen eingeben: ");	//read username into dynamic buffer
	fflush(stdout);
	username = read_input();

	system("stty -echo");

	printf("Passwort eingeben: "); //read password
	fflush(stdout);
	password = read_input();
	printf("\n");

	system("stty echo");

	struct string read_write_send = { .data = malloc(4), .length = 4, .capacity = 4};

	memcpy(read_write_send.data, "\x00\x02" "1\0", 4); //prepare String to be send to the server
	read_write_send.data[2] = read_write;

	send_string(&read_write_send, *socket_fd_pointer);
	send_string(username, *socket_fd_pointer);
	send_string(password, *socket_fd_pointer);

	get_message(&answer, *socket_fd_pointer);

	if(answer.length != 2 || answer.data[0] == '0')
	{
		printf("Anmeldung abgwiesen vom Server (Falscher Benutzername oder Passwort?)\n");
		exit(1);
	}

	signal(SIGINT, sigint_handler);
	pthread_create(&interface_thread, NULL, interface_thread_func, socket_fd_pointer); //start interface thread

	free(read_write_send.data);
	free(password->data);
	free(password);				//cleanup
	free(username->data);
	free(username);
	free(home_server->data);
	free(home_server);
	free(answer.data);

	pthread_join(interface_thread, NULL);
	free(socket_fd_pointer); //free socket pointer *after* threads are done
	return 0;
}

static struct string* read_input(void)		//allocate some space and read byte by byte, realloc of necessary
{
	struct string* user_input = malloc(sizeof(*user_input));
	user_input->data = malloc(DEFAULT_NAME_LENGTH);
	user_input->length = 0;
	user_input->capacity = DEFAULT_NAME_LENGTH;

	int c;

	while((c = getchar()) != '\n' && c != EOF)
	{
		realloc_write(user_input, (char)c, user_input->length);
		user_input->length++;
	}
	user_input->data[user_input->length] = '\0';
	user_input->length++;
	convert_string(user_input);


	return user_input;
}

static void sigint_handler(int signum) //set global vaiable for reading, instead of crashing the application
{
	signal(SIGINT, sigint_handler);
	interrupted = 1;
}

static int* init_connection(struct string* home_server)
{
	int* socket_fd_pointer = malloc(sizeof(*socket_fd_pointer));
	int status;
	struct addrinfo hints;
	struct addrinfo *res;  				// will point to the results of getaddrinfo()

	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_INET;     		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; 	// TCP stream sockets

	if ((status = getaddrinfo(home_server->data+2, "2000", &hints, &res)) != 0) //get ip from hostname
	{
		printf("getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	if((*socket_fd_pointer = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)	//create socket
	{
		printf ("Socket konnte nicht angelegt werden: %s\n", strerror(errno));
		exit(1);
	}

	if(connect(*socket_fd_pointer, res->ai_addr, res->ai_addrlen) == -1)		//connect to server
	{
		printf ("Verbindung mit dem Master-Server konnte nicht hergestellt werden: %s\n", strerror(errno));
		exit(1);
	}

	freeaddrinfo(res);

	return socket_fd_pointer;
}
