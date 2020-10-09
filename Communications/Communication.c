/*
 *
 * Communication Message Passing
 *
 *  Created on: 2 October 2020
 *      Author: Adam Hawke
 *      Last Modified: 2/10/2020
 */

/*
 * *******************************************
 *
 * 				Included Files
 *
 *
 * *******************************************
 */

#include <stdio.h>
#include <string.h>
#include <sys/dispatch.h>

#include "Communication.h"

/*
 * *******************************************
 *
 * 				Functions/Threads
 * 				  Definitions
 *
 * *******************************************
 */

int setpriority(t_priority * th){
	// Configuring Threads
	pthread_attr_init (&th->th_attr);										// Initialise thread attribute object to the default values (required)
	pthread_attr_setschedpolicy (&th->th_attr, SCHED_RR);					// Explicitly set the scheduling policy to round robin
	th->th_param.sched_priority = th->priority;								// Set thread priority (can be from 1..63 in QNX, default is 10)
	pthread_attr_setschedparam (&th->th_attr, &th->th_param);
	pthread_attr_setinheritsched (&th->th_attr, PTHREAD_EXPLICIT_SCHED);	// Now set attribute to use the explicit scheduling settings
	pthread_attr_setstacksize (&th->th_attr, 8000);							// Increase the thread stacksize
	return 1;
}

// Function defined by state machine developer
int incomingDataHandler(char a)
{

	//pthread_mutex_lock(&sm_data);
	//sm_data->num = a;
	//pthread_mutex_unlock(&sm_data);

	printf("Received: %c on Attach Point %s through DataHandler\n",a,ATTACH_POINT);
	return 2;
}

int sendHandler(char msg, int len,void *Data, int server)
{
	client_data *data = (client_data*)Data;

	printf("...sending message from handler: %c\n",msg);
	pthread_mutex_lock(&data->client_mutex);
	data->input = msg;
	data->client_ready = 1;							// Notify Client Thread Message is Ready
	data->server_id = server;
	//pthread_cond_signal(&data->client_condvar);	// Signal/Wake-up key scanner thread
	pthread_mutex_unlock(&data->client_mutex);		// Unlock Mutex
	//sched_yield();								// Change priority to main thread

	return 0;
}

// Contantly Scan server for new input
void * recievedata(void *Data){
	server_data *data = (server_data*)Data;
	char msg;
    while(1){
        // Wait here to receive data from server
        if(data->server_ready == 1){
	        // Lock Server Mutex
	        pthread_mutex_lock(&data->server_mutex);

	        msg = data->server_msg;  	// Consume Message
	        data->server_ready = 0;		// Note data has been consumed

            // Wait for more data and signal Server Thread
	        //pthread_cond_signal(&data->server_condvar);	// Signal/Wakeup key scanner thread
	        pthread_mutex_unlock(&data->server_mutex);	// Unlock Mutex
	        //sched_yield();  									// Change priority to Server

            // Send data to Handler
            incomingDataHandler(msg);
	    }
        // Wait for more data
    }
}

// Function defined by message passing / comms developer
void * message_init(void *ptr,server_data * s_data,client_data * c_data)
{
    // Create Server Thread
    t_priority th1 = {15};
    setpriority(&th1);
    t_priority th2 = {10};
	setpriority(&th2);
	t_priority th3 = {12};
	setpriority(&th3);

	// Spawn threads for server, receive handler and client
    pthread_create(&th1.th,&th1.th_attr,server,s_data);
	pthread_create(&th2.th,&th2.th_attr,recievedata,s_data);
	pthread_create(&th3.th,&th3.th_attr,client,c_data);

    // Return Method of Sending Data
	return &sendHandler;
}

/*
 * *******************************************
 *
 * 				 Server Thread
 *
 *
 * *******************************************
 */
void *server(void *Data)
{
	printf("-> Server thread started... \n");

	// Initalising variables
	name_attach_t *attach;
	server_data *data = (server_data*) Data;
	my_data msg;
	my_reply replymsg; // replymsg structure for sending back to client

	replymsg.hdr.type = 0x01;       // some number to help client process reply msg
	replymsg.hdr.subtype = 0x00;    // some number to help client process reply msg

	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL)
	{
		printf("\nFailed to name_attach on ATTACH_POINT: %s \n", ATTACH_POINT);
  	    printf("\n Possibly another server with the same name is already running or you need to start the gns service!\n");
  	    return (int*)EXIT_FAILURE;
	}

	printf("-> Server Listening for Clients on ATTACH_POINT: %s \n\n", ATTACH_POINT);

	// Server Loop
	int rcvid=0, msgnum=0;  		// no message received yet
	int Stay_alive=0, living=0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)
	living = 1;
	while (living)
	{
		// Do your MsgReceive's here now with the chid
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1)  // Error condition, exit
		{
			printf("\nFailed to MsgReceive\n");
			break;
		}

	   // Decide if a Pulse or message was received
	   // for Pulses:
	   if (rcvid == 0)  //  Pulse received, work out what type
	   {
		   //printf("\nServer received a pulse from ClientID:%d ...\n", msg.ClientID);
		   //printf("Pulse received:%d \n", msg.hdr.code);

		   switch (msg.hdr.code)
		   {
			   case _PULSE_CODE_DISCONNECT:
				   //printf("Pulse case:    %d \n", _PULSE_CODE_DISCONNECT);
					// A client disconnected all its connections by running
					// name_close() for each name_open()  or terminated
				   if( Stay_alive == 0)
				   {
					   printf("\nServer Detached from ClientID:%d ...\n", msg.ClientID);
					   ConnectDetach(msg.ClientID);
					   //living = 0; // kill while loop
					   break;
				   }
				   else
				   {
					   printf("\nServer received Detach pulse from ClientID:%d but rejected it ...\n", msg.ClientID);
				   }
				   break;

			   case _PULSE_CODE_UNBLOCK:
					// REPLY blocked client wants to unblock (was hit by a signal
					// or timed out).  It's up to you if you reply now or later.
				   printf("\nServer got _PULSE_CODE_UNBLOCK after %d, msgnum\n", msgnum);
				   break;

			   case _PULSE_CODE_COIDDEATH:  // from the kernel
				   //printf("\nServer got _PULSE_CODE_COIDDEATH after %d, msgnum\n", msgnum);
				   break;

			   case _PULSE_CODE_THREADDEATH: // from the kernel
				   printf("\nServer got _PULSE_CODE_THREADDEATH after %d, msgnum\n", msgnum);
				   break;

			   default:
				   // Some other pulse sent by one of your processes or the kernel
				   printf("\nServer got some other pulse after %d, msgnum\n", msgnum);
				   break;

		   }
		   continue;// go back to top of while loop
	   }

	   // for messages:
	   if(rcvid > 0) // if true then A message was received
	   {
		   msgnum++;

		   // If the Global Name Service (gns) is running, name_open() sends a connect message. The server must EOK it.
		   if (msg.hdr.type == _IO_CONNECT )
		   {
			   MsgReply( rcvid, EOK, NULL, 0 );
			   printf("\nClient messaged indicating that GNS service is running....");
			   printf("\n    -----> replying with: EOK\n");
			   msgnum--;
			   continue;	// go back to top of while loop
		   }

		   // Some other I/O message was received; reject it
		   if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX )
		   {
			   MsgError( rcvid, ENOSYS );
			   printf("\n Server received and IO message and rejected it....");
			   continue;	// go back to top of while loop
		   }

		   // A message Recieved
		   pthread_mutex_lock(&data->server_mutex);

		   //printf("Server Received Msg... %c \n", msg.data);
		   data->server_msg = msg.data;

		   /* Signaling and Flow Control */
		   data->server_ready = 1;						// Notify data has been produced
		   //pthread_cond_signal(&data->server_condvar);	// Signal/Wakeup key scanner thread
		   pthread_mutex_unlock(&data->server_mutex);	// Unlock Mutex
		   //sched_yield();


		   sprintf(replymsg.buf, "Message %d received", msgnum);
		   printf("Server received data packet with value of '%c' from client (ID:%d)\n", msg.data, msg.ClientID);
		   fflush(stdout);
		   // sleep(1); // Delay the reply by a second (just for demonstration purposes)

		   //printf("\n    -----> replying with: ACK\n");
		   MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
	   }
	   else
	   {
		   printf("\nERROR: Server received something, but could not handle it correctly\n");
	   }

   }

   // Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
   name_detach(attach, 0);

   return EXIT_SUCCESS;
}

/*
 * *******************************************
 *
 * 				Client Threads
 *
 *
 * *******************************************
 */

void* client(void* Data)
{
	printf("-> Client thread started...");
	client_data* data = (client_data*)Data;

    my_data msg;				// msg structure for sending from client
    my_reply reply;				// replymsg structure for sending back to client

    msg.ClientID = CLIENT_ID;	// unique number for this client
    msg.hdr.type = 0x22;    	// We would have pre-defined data to stuff here

    int server_id1;				// Server id connection 1
    int server_id2;				// Server id connection 2
    int no_data = 0;			// Data recieved flag
    int server = 0;				// Server to send to

    int err_node1 = 1;			// Stores the state of the connection to node 1
    int err_node2 = 1;			// Stores the state of the connection to node 2

    // Prepare attach point string
    char ATTACH_POINT_1[100];
    char ATTACH_POINT_2[100];

    strcpy(ATTACH_POINT_1, "/net/");
    strcat(ATTACH_POINT_1, I1_ATTACH_DEVICE);
    strcat(ATTACH_POINT_1, "/dev/name/local/");
    strcat(ATTACH_POINT_1, I1_ATTACH_POINT);

    // Configure second channel if present
	if(NUMBER_CLIENT == 2){
		strcpy(ATTACH_POINT_2, "/net/");
		strcat(ATTACH_POINT_2, I2_ATTACH_DEVICE);
		strcat(ATTACH_POINT_2, "/dev/name/local/");
		strcat(ATTACH_POINT_2, I2_ATTACH_POINT);
	}

    // Constantly poll the connection
    while(1){
    	// Error check for attach point, if not found keep trying to reconnect
    	if((server_id1 = name_open(ATTACH_POINT_1, 0)) == -1){
    		if(err_node1 == 0){
    		printf("\n    ERROR, connection lost to: %s \n    Trying to reconnect!\n\n",I1_ATTACH_POINT);
    			err_node1 = 1;	// Set Error
    		}
    	}
    	else{
    		if(err_node1 == 1){
    		printf("\nConnection re-established to: %s \n\n",I1_ATTACH_POINT);
    		err_node1 = 0;		// Clear Error
    		}
    	}

    	// Error check for attach point, if not found keep trying to reconnect
		if(NUMBER_CLIENT == 2){
			if((server_id2 = name_open(ATTACH_POINT_2, 0)) == -1){
				if(err_node2 == 0){
					// Notify Console
					printf("\n    ERROR, connection lost to: %s \n    Trying to reconnect!\n\n",I2_ATTACH_POINT);
						err_node2 = 1;	// Set Error
				}
			}
			else{
				if(err_node2 == 1){
					// Notify Console
					printf("\nConnection re-established to: %s \n\n",I2_ATTACH_POINT);
					err_node2 = 0;		// Clear Error
				}
			}
		}

    	// set up data packet
    	pthread_mutex_lock(&data->client_mutex);
    	no_data = data->client_ready;				// Store if data is present
    	if (data->client_ready == 1) {
    		msg.data = data->input;					// Store Data
    		data->client_ready = 0;					// Notify data has been consumed
    		server = data->server_id;
    	}
    	pthread_mutex_unlock(&data->client_mutex);	// Unlock Mutex

		fflush(stdout);	// Clear the output buffer

		// the data we are sending is in msg.data


		// ADD ERROR CHECKING FOR WHICH NODE TO SEND DATA
		if(server == 1){
			// Send data if server is connected
			if((no_data == 1) && (err_node1 == 0)){

				printf("...sending message from client: %c, to &s\n",msg.data,I1_ATTACH_POINT);

				// Send to Server ID1
				if (MsgSend(server_id1, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
				{
					printf(" Error data '%d' NOT sent to server\n", msg.data);
						// maybe we did not get a reply from the server
					break;
				}
				else
				{ 	// now process the reply
					printf("   -->Reply is: '%s'\n", reply.buf);
				}
			}
		}
		else if(server == 2){
			// Send data if server is connected
			if((no_data == 1) && (err_node2 == 0)){

				printf("...sending message from client: %c, to %s\n",msg.data,I2_ATTACH_POINT);

				// Send to Server ID1
				if (MsgSend(server_id2, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
				{
					printf(" Error data '%d' NOT sent to server\n", msg.data);
						// maybe we did not get a reply from the server
					break;
				}
				else
				{ 	// now process the reply
					//printf("   -->Reply is: '%s'\n", reply.buf);
				}
			}
		}
        //usleep(1);	// wait a few seconds before sending the next data packet
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server_id1);
    name_close(server_id2);

    return EXIT_SUCCESS;
}
