#include "SFM_Client.h"

static void print_formatted_message(struct string* message);
static bool handle_server_command(char* const command);
static void copy_until_delimiter_offset(struct string* restrict source, struct string* restrict target,char delimiter);

void* listen_thread_func(void*  arg)
{
	struct string message = new_string(DEFAULT_BUFFER_LENGTH);
	struct return_info return_codes;
	sigset_t   signal_mask;
	sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGINT);
    pthread_sigmask (SIG_BLOCK, &signal_mask, NULL); //block SIGINT

	int* socket_fd_pointer = (int*)arg;

get_message:
	return_codes = get_message(&message, *socket_fd_pointer);
	if(!return_codes.error_occured)
	{
		if(!return_codes.return_code)
		{
			printf("Keine Nachrichten verfügbar\n");
			fflush(stdout);
		} else if(message.data[0] == '\0') {
			goto get_message;
		} else if(!handle_server_command(message.data)) {  //check if message was a command, otherwise its a message
			print_formatted_message(&message);
			fflush(stdout);
		}
	} else {
		printf("Fehler beim holen der Nachrichten\n");
		fflush(stdout);
	}

	free(message.data);
	pthread_exit(0);
}

static void print_formatted_message(struct string* message) //print message in a readable format
{
	uint32_t group_offset;
	bool contains_group = false;

	struct string servername = new_string(DEFAULT_NAME_LENGTH);
	struct string username = new_string(DEFAULT_NAME_LENGTH);
	char* timestamp = malloc(200);
	struct string temp_message = new_string(DEFAULT_NAME_LENGTH);
	message->length = 0;

	message->length += 8;		//8 bits of time
	int64_t time = *(int64_t*)message->data;
	strftime(timestamp, 200, "%d-%m-%Y %H:%M:%S", localtime((time_t*)&time));
	message->length += 1; 		//'>'
	copy_until_delimiter_offset(message,&servername,'@');
	copy_until_delimiter_offset(message,&username,':');
	copy_until_delimiter_offset(message,&temp_message,'\0');

	for(group_offset = 0; group_offset < strlen(username.data); group_offset++) // find out if message was send to group
	{
		if(username.data[group_offset] == '/')
		{
			username.data[group_offset] = '\0'; //replace '/' with nul to print out easily with printf
			contains_group = true;
			break;
		}
	}

	printf("\n\tVon: %s@%s\n", servername.data, username.data);

	if(contains_group)
		printf("\t Gruppe: %s\n", username.data+group_offset+1);

	printf("\tGesendet am: %s\n", timestamp);
	printf("\tNachricht:\n%s\n\n", temp_message.data);

	free(servername.data);
	free(username.data);
	free(temp_message.data);
	free(timestamp);
}

static void copy_until_delimiter_offset(struct string* restrict source, struct string* restrict target,char delimiter)
{
	uint32_t offset_target;
	for(offset_target = 0; source->data[source->length] != delimiter; offset_target++, source->length++) 	//go through source and copy the chars from offset_source
		realloc_write(target, source->data[source->length], offset_target);									//until the delimiter to the target

	source->length++;
	target->data[offset_target] = '\0';											//skip delimiter so you can call this function directly again, and nul-terminate string
}

static bool handle_server_command(char* const command) //returns true and handles command if it was a command, otherwise returns false
{
	if(command[0] != '/')
		return false;
	else if(strncmp(command+1, "motd", strlen("motd")) == 0)
		printf("MOTD: %s\n", command+strlen("/motd "));
	else if(strncmp(command+1, "misc", strlen("misc")) == 0)
		printf("Nachricht vom Server:\n%s\n\n", command+strlen("/misc "));

	else
		printf("Unbekannter Befehl vom Server erhalten: %s\n", command);

	fflush(stdout);
	return true;
}

