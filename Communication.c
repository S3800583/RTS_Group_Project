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
int incomingDataHandler(int a)
{
	getMutex()
	// change parameters
	release mutex()
	
	printf("From my_func, a = %d\n",a);
	return 2;
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
