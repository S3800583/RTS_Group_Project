#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "Communication.h"

int main(void) {
	printf("-> Starting main thread...\n");

	server_data s_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0};
	client_data c_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,0};

	int (*send)(char, int, void *);
	send = message_init(&incomingDataHandler,&s_data,&c_data);

	sleep(10);
	sleep(10);

	char test = 0;
	printf("Enter Character to send...\n");
	scanf(" %c", test);
	send(test,sizeof(test),&c_data);

	sleep(10);
	sleep(10);
	sleep(10);

	puts("\n\nExiting main thread...."); /* prints Hello World!!! */
	return EXIT_SUCCESS;
}
