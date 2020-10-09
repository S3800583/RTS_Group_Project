
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "Communication.h"

typedef struct{
	int (*send)(char, int, void *,int);
}Comms;

typedef struct
{
	pthread_mutex_t mutex; 	// needs to be set to PTHREAD_MUTEX_INITIALIZER;
	int data_ready, server_id;
	char input[5];

	// Communications
	Comms d;
	client_data c_data;
}Mydata;

void *sensor(void *Data);

// Function defined by state machine developer
int incomingDataHandler(char a);


int main(void){
	printf("-> Starting main thread...\n");
	Mydata my_data = {PTHREAD_MUTEX_INITIALIZER,0,0,{0},
			.c_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,0}};

	server_data s_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0};

	char msg = 0;
	char c1status[5];
	int server,c_connection = 0;
	int senddata = 0;
	my_data.d.send = message_init(&incomingDataHandler,&s_data,&my_data.c_data);

	// Create Server Thread
	t_priority th1 = {16};
	setpriority(&th1);

	// Spawn threads for server, receive handler and client
	pthread_create(&th1.th,&th1.th_attr,sensor,&my_data);


	while(1){

		if((my_data.data_ready == 1) || (my_data.c_data.c1_status == 1)){
		pthread_mutex_lock(&my_data.mutex);			// Request Command
			msg = my_data.input[0];
			server = my_data.server_id;
			my_data.data_ready = 0;
			senddata = 1;
			c_connection = my_data.c_data.c1_status;
			//my_data.c_data.c2_status
		pthread_mutex_unlock(&my_data.mutex);		// Unlock Mutex
		}

		if(c_connection == 1){
			strcpy(c1status,"UP");
		}
		else if(c_connection == 0){
			strcpy(c1status,"DOWN");
		}

		printf(	"\fTrain Connection: %s\t Train Status: %s\n", c1status);

		if(senddata == 1){


			my_data.d.send(msg,sizeof(msg),&my_data.c_data,server);
			senddata = 0;
		}

	}

	puts("\n\nExiting main thread...."); /* prints Hello World!!! */
	return EXIT_SUCCESS;
}

// Function defined by state machine developer
int incomingDataHandler(char a)
{
	//char t_status;
	//char t_state;

	//printf(	"Train Connection: %s\t Train Status: %s\n", c_status,c_state);

	printf("Received: %c on Attach Point %s through DataHandler\n",a,ATTACH_POINT);
	return 2;
}

void *sensor(void *Data){

    printf("\n-> Sensor thread started...\n");

    /* Initializing Data */
    Mydata *data = (Mydata*) Data;

    // Commands
    /* Scan for input from keyboard*/
    while(1){
    		pthread_mutex_lock(&data->mutex);
    												// Request Command
    		while(data->data_ready == 0){
    			//printf("\nCR> ");
    			scanf("%[^\n]%*c",data->input);			// Scan input with spaces

				if(!(strcmp(data->input,"I1 P"))){		// Compare to known commands
					data->input[0] = 'P';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I1 O"))){
					data->input[0] = 'O';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I1 b"))){
					data->input[0] = 'b';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I2 p"))){	// Compare to known commands
					data->input[0] = 'p';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 2;			// Select Server
				}
				else if(!(strcmp(data->input,"I2 o"))){
					data->input[0] = 'o';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 2;			// Select Server
				}
				else if(!(strcmp(data->input,"I2 b"))){
					data->input[0] = 'b';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 2;			// Select Server
				}
				else{
					printf("Error: Command Unknown...\n");
					data->input[0] = '\0';				// Clear Received input
					data->data_ready = 0;				// Re-Scan Keyboard
				}
    		}

			pthread_mutex_unlock(&data->mutex);		// Unlock Mutex
    }
}
