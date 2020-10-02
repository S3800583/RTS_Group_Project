/*
 * Communication.h
 *
 *  Created on: 2Oct.,2020
 *      Author: Adam
 */

#ifndef SRC_COMMUNICATION_H_
#define SRC_COMMUNICATION_H_

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

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

/*
 * *******************************************
 *
 * 				  Definitions
 *
 *
 * *******************************************
 */

#define BUF_SIZE 100
#define ATTACH_POINT "TrafficController1"  // This must be the same name that is used for the client.

/*
 * *******************************************
 *
 * 			Structs and Data Types
 *
 *
 * *******************************************
 */

typedef union {	  			// This replaced the standard:  union sigval
	union{
		_Uint32t sival_int;
		void *sival_ptr;	// This has a different size in 32-bit and 64-bit systems
	};
	_Uint32t dummy[4]; 		// Hence, we need this dummy variable to create space
}_mysigval;

typedef struct _Mypulse {   // This replaced the standard:  typedef struct _pulse msg_header_t;
   _Uint16t type;
   _Uint16t subtype;
   _Int8t code;
   _Uint8t zero[3];         // Same padding that is used in standard _pulse struct
   _mysigval value;
   _Uint8t zero2[2];		// Extra padding to ensure alignment access.
   _Int32t scoid;
} msg_header_t;

typedef struct {
   msg_header_t hdr;  // Custom header
   int ClientID;      // our data (unique id from client)
   int data;          // our data <-- This is what we are here for
} my_data;

typedef struct {
   msg_header_t hdr;   // Custom header
   char buf[BUF_SIZE]; // Message to send back to send back to other thread
} my_reply;

/* Initialising Threads Mutex and Conditional Variable */
typedef struct
{
	pthread_mutex_t server_mutex; 	// needs to be set to PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t server_condvar; // needs to be set to PTHREAD_COND_INITIALIZER;
	int server_ready;
	char server_msg;
}server_data;

/*
 * *******************************************
 *
 * 				Functions/Threads
 * 				  Definitions
 *
 * *******************************************
 */

// Function defined by state machine developer
int incomingDataHandler(int a);

// Retrieve Message from Server Thread
int recieveHandler(void *data, int len,server_data * server_data);

// Function defined by message passing / comms developer
void * message_init(void *ptr);

void *server(void *Data);

#endif /* SRC_COMMUNICATION_H_ */
