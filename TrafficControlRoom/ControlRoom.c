
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



int main(void){
	printf("-> Starting main thread...\n");
	Mydata my_data = {PTHREAD_MUTEX_INITIALIZER,0,0,{0},
			.c_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,0}};

	server_data s_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0};

	char msg = 0;
	int server = 0;
	int senddata = 0;
	my_data.d.send = message_init(&incomingDataHandler,&s_data,&my_data.c_data);

	// Create Server Thread
	t_priority th1 = {16};
	setpriority(&th1);

	// Spawn threads for server, receive handler and client
	pthread_create(&th1.th,&th1.th_attr,sensor,&my_data);


	while(1){
		pthread_mutex_lock(&my_data.mutex);
		    												// Request Command
		if(my_data.data_ready == 1){
			msg = my_data.input[0];
			server = my_data.server_id;
			my_data.data_ready = 0;
			senddata = 1;
		}

		pthread_mutex_unlock(&my_data.mutex);		// Unlock Mutex
		if(senddata == 1){
			my_data.d.send(msg,sizeof(msg),&my_data.c_data,server);
			senddata = 0;
		}
	}

	puts("\n\nExiting main thread...."); /* prints Hello World!!! */
	return EXIT_SUCCESS;
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

				if(!(strcmp(data->input,"I1 p"))){		// Compare to known commands
					data->input[0] = 'p';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I1 o"))){
					data->input[0] = 'o';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I1 t"))){
					data->input[0] = 't';				// Prepare corresponding message
					data->data_ready = 1;				// Notify Client Thread Message is Ready
					data->server_id = 1;			// Select Server
				}
				else if(!(strcmp(data->input,"I1 u"))){
					data->input[0] = 'u';				// Prepare corresponding message
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
