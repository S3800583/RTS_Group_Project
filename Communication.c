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
 * 				  Definitions
 *
 *
 * *******************************************
 */

/*
 * *******************************************
 *
 * 			Structs and Data Types
 *
 *
 * *******************************************
 */

/* Initialising Threads Mutex and Conditional Variable */
typedef struct
{
	char msg_data;
}app_data;

/*
 * *******************************************
 *
 * 		Functions/Threads Prototypes
 *
 *
 * *******************************************
 */

int incomingDataHandler(int a);

int recieveHandler(void *data, int len, server_data * server_data);
// Function defined by message passing / comms developer
void * message_init(void *ptr);


/*
 * *******************************************
 *
 * 				Main Function
 *
 * *******************************************
 */

/*
 * *******************************************
 *
 * 				Functions/Threads
 * 				  Definitions
 *
 * *******************************************
 */

// Function defined by state machine developer
int incomingDataHandler(char a)
{
	//getMutex()
	// change parameters
	//release mutex()
	
	printf("From my_func, a = %d\n",a);
	return 2;
}

int sendHandler(void *data, int len)
{
	
	printf("will send %s of size %d\n",(char *)data,len );
	return 0;
}

// Contantly Scan server for new input
void * recivedata(void *Data){
    while(1){
        // Wait here to receive data from server
        if(server_data->server_ready == 1){
	        // Lock Server Mutex
	        pthread_mutex_lock(&server_data->server_mutex);

	        msg = server_data->server_msg;  // Consume Message
	        server_data->server_ready = 0;	// Note data has been consumed

            // Wait for more data and signal Server Thread
	        pthread_cond_signal(&server_data->server_condvar);	// Signal/Wakeup key scanner thread
	        pthread_mutex_unlock(&server_data->server_mutex);	// Unlock Mutex
	        sched_yield();  									// Change priority to Server

            // Send data to Handler
            incomingDataHandler(msg);
	    }
        // Wait for more data
    }
}

// Function defined by message passing / comms developer
void * message_init(void *ptr,server_data * server_data)
{
    // Create Server Thread
    pthread_t th1,th2;
    pthread_create(&th1,NULL,&server,&server_data);
	pthread_create(&th2,NULL,&recievedata,&server_data);
    // Point Function to Incoming Handler
    int (*function)(char);
	function = ptr;

	// wait here to recieve data	
	// int b = function(10);						// calling the function defined by state machine developer when respective data is recieved
	// printf("from init = %d\n", b);

    // Return Method of Sending Data
	return &sendHandler;
}

int main(int argc, char const *argv[])
{
	printf("Main start!!!\n");
	int (*send)(void *, int);
	send = message_init(&incomingDataHandler,&server_data);		// function registration
	send("abcd",strlen("abcd"));			        			// data and its length
	printf("Main end!!!\n");
	return 0;
}








// Retrieve Message from Server Thread
int recieveHandler(void *data, int len,server_data * server_data){
	char msg;
	if(server_data->server_ready == 1){
	// Wait here to receive data from server
	pthread_mutex_lock(&server_data->server_mutex);

	// Consume Message
	msg = server_data->server_msg;

	/* Otherwise wait for more data */
	server_data->server_ready = 0;						// Notify data has been consumed
	pthread_cond_signal(&server_data->server_condvar);	// Signal/Wakeup key scanner thread
	pthread_mutex_unlock(&server_data->server_mutex);	// Unlock Mutex
	sched_yield();										// Change priority to Server
	}

	//printf("Nothing received, going to sleep...\n");
	//sleep(1);
	return msg;
}
recieve(){
	while(1)
		{
		connect server

		wait for server to send something

		post processing,

		incomingDataHandler(data);
	}
}
// Function defined by message passing / comms developer
void * message_init(void *ptr){
	int (*foo)(int);
	foo = ptr;
	// wait here to recieve data
	int b = foo(10);						// calling the function defined by state machine developer when respective data is recieved
	printf("from init = %d\n", b);
	return &recieveHandler;
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
	//printf("_mysigval size= %d\n", sizeof(_mysigval));
	//printf("header size= %d\n", sizeof(msg_header_t));
	//printf("my_data size= %d\n", sizeof(my_data));
	//printf("my_reply size= %d\n", sizeof(my_reply));

   name_attach_t *attach;
   server_data *data = (server_data*) Data;
   my_data msg;
   my_reply replymsg; // replymsg structure for sending back to client

   replymsg.hdr.type = 0x01;       // some number to help client process reply msg
   replymsg.hdr.subtype = 0x00;    // some number to help client process reply msg

   // Create a global name (/dev/name/local/...)
   if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL)
   // Create a global name (/dev/name/global/...)
   //if ((attach = name_attach(NULL, ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL)) == NULL)
   {
       printf("\nFailed to name_attach on ATTACH_POINT: %s \n", ATTACH_POINT);
       printf("\n Possibly another server with the same name is already running or you need to start the gns service!\n");
	   return EXIT_FAILURE;
   }

   printf("-> Server Listening for Clients on ATTACH_POINT: %s \n\n", ATTACH_POINT);

   	/*
	 *  Server Loop
	 */
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

       // did we receive a Pulse or message?
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
					   //ConnectDetach(msg.hdr.scoid);
					   //printf("\nServer was told to Detach from ClientID:%d ...\n", msg.ClientID);
					   //printf("Server did not disconnect...\n\n");
					   living = 0; // kill while loop
					   continue;
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
				   printf("\nServer got _PULSE_CODE_COIDDEATH after %d, msgnum\n", msgnum);
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

		   // A message (presumably ours) received
		   /* Producer Loop */
		   pthread_mutex_lock(&data->server_mutex);
		   // put your message handling code here and assemble a reply message

		   //printf("Produce Msg... %c \n", msg.data);
		   data->server_msg = msg.data;

		   /* Signaling and Flow Control */
		   data->server_ready = 1;					// Notify data has been produced
		   pthread_cond_signal(&data->server_condvar);	// Signal/Wakeup key scanner thread
		   pthread_mutex_unlock(&data->server_mutex);	// Unlock Mutex
		   sched_yield();


		   sprintf(replymsg.buf, "Message %d received", msgnum);
		   //printf("Server received data packet with value of '%d' from client (ID:%d), ", msg.data, msg.ClientID);
		   fflush(stdout);
		   // sleep(1); // Delay the reply by a second (just for demonstration purposes)

		   //printf("\n    -----> replying with: '%s'\n",replymsg.buf);
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

void * client(void *Data)
{
	printf("-> Client thread started...\n");

	// Initialising Variables
	client_data *data = (client_data*) Data;
    my_data msg;
    my_reply reply; 		 	// replymsg structure for sending back to client
    msg.ClientID = 551;      	// unique number for this client
    msg.hdr.type = 0x22;     	// We would have pre-defined data to stuff here
    int server_I1;
    int server_I2;
    int server_id;

    // Open Channels
    if ((server_I1 = name_open(I1_ATTACH_POINT, 0)) == -1){
        printf("\n    ERROR, could not connect to server: Traffic Controller 1\n\n");
        //return EXIT_FAILURE;
    }
    else{
    	printf("Connection established to: Traffic Controller 1\n");
    }

    if ((server_I2 = name_open(I2_ATTACH_POINT, 0)) == -1){
    	printf("\n    ERROR, could not connect to server: Traffic Controller 2\n\n");
        //return EXIT_FAILURE;
    }
    else{
    	printf("Connection established to: Traffic Controller 2\n");
    }

    // Send Messages
    while(1){ // send data packets
    	/* Consumer Loop */
    	pthread_mutex_lock(&data->client_mutex);

    	if(data->client_ready == 1){
    		/* Displaying Data Received from Sensor*/
    		//printf("Received Input: %c \n", data->input[0]);	// Display input
    		msg.data = data->input[0];				// Store Data
    		server_id = data->server_id;			// Store Server ID
    		/* Signaling and Flow Control */
    		data->client_ready = 0;					// Notify data has been consumed
    	}

    	pthread_cond_signal(&data->client_condvar);	// Signal/Wakeup key scanner thread
    	pthread_mutex_unlock(&data->client_mutex);		// Unlock Mutex
    	sched_yield();

    	// the data we are sending is in msg.data
        // Determine Who the message was for
    	fflush(stdout);
    	if(server_id == 1){
    		if (MsgSend(server_I1, &msg, sizeof(msg), &reply, sizeof(reply)) == -1){
    			printf(" Error data '%d' NOT sent to server\n", msg.data);
    			// maybe we did not get a reply from the server
    			return 0;
    		}
    		else{ 	// now process the reply
    			//printf("   -->Reply is: '%s'\n", reply.buf);
    		}
    	}
    	else if(server_id == 2){
    		if (MsgSend(server_I2, &msg, sizeof(msg), &reply, sizeof(reply)) == -1){
				printf(" Error data '%d' NOT sent to server\n", msg.data);
				// maybe we did not get a reply from the server
				return 0;
			}
    		else{	// now process the reply
    			//printf("   -->Reply is: '%s'\n", reply.buf);
    		}
    	}
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server_I1);

    return EXIT_SUCCESS;
}

int client_send(client_data *Data,char input){
	// Initialising Variables
	client_data *data = (client_data*) Data;
	int ret = 0;

	pthread_mutex_lock(&data->client_mutex);

	data->client_ready = 1;				// Notify Client Thread Message is Ready

	pthread_cond_signal(&data->client_condvar);		// Signal/Wake-up key scanner thread
	pthread_mutex_unlock(&data->client_mutex);		// Unlock Mutex
	sched_yield();									// Change priority to main thread

	ret = 1;

	return ret;
}
