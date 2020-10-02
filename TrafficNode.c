#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#include "Communication.h"

#define size 10
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL


enum states { state0, state1, state2, state3, state4, state5, state6, state7, state8, state9,
			  state10, state11, state12, state13, state14, state15, state16, state17,
			  state18, state19, state20
			};



typedef struct
{
	pthread_mutex_t 		mutex;
	pthread_cond_t 			s;

	int 					ew, ns, add, tmp, ret;
	int						L1, L2, L3, L4, sp, wp, np, ep,
							pt, ot, oL1, oL2, oL3, oL4,
							ews, ewr, nsr, osp, owp, onp, oep,
							sm1, sm2;
	int                     chid, rcvid, fd;
	char 					num;
	char 					buf[size];
	timer_t                 timer_id;
	struct sigevent         event;
	struct itimerspec      	red;
	struct itimerspec      	green;
	struct itimerspec      	yellow;
	struct _pulse  			pulse;
	struct pollfd 			fds[1];
	enum 					states currState;
}Mydata;

typedef struct{
	server_data s_data;
	Mydata m_data;
}anyname;

	Mydata msg;

void *timer(void *data)
{
	Mydata *var = (Mydata*)data;

		var->chid = ChannelCreate(0); // Create a communications channel
		var->event.sigev_notify = SIGEV_PULSE;
		// create a connection back to ourselves for the timer to send the pulse on
		var->event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, var->chid, _NTO_SIDE_CHANNEL, 0);
		var->event.sigev_code = MY_PULSE_CODE;

		// create the timer, binding it to the event
		if (timer_create(CLOCK_REALTIME, &var->event, &var->timer_id) == -1)
		{
		   //printf (stderr, "%s:  couldn't create a timer, errno %d\n", progname, errno);
		   perror (NULL);
		   exit (EXIT_FAILURE);
		}

		// setup the timer (1s initial delay value, 1s reload interval)


		var->red.it_value.tv_sec = 1;			  // 1 second
		var->red.it_interval.tv_sec = 1;          // 1 second

			var->green.it_value.tv_sec = 3;			  // 1 second
			var->green.it_interval.tv_sec = 3;          // 1 second

				var->yellow.it_value.tv_sec = 2;			  // 1 second
				var->yellow.it_interval.tv_sec = 2;          // 1 second


		// and start the timer!
		timer_settime(var->timer_id, 0, &var->red, NULL);
		timer_settime(var->timer_id, 0, &var->green, NULL);
		timer_settime(var->timer_id, 0, &var->yellow, NULL);



}

void *server(void *Data);
int incomingDataHandler(int a);


void *sensor(void *data)
{

	Mydata *var = (Mydata*)data;

	while(1)
	{
		var->fds[0].fd = var->fd;

			memset((void * ) var->buf, 0, size); // buffer created

		var->ret = read(var->fd,(void *)var->buf, size); // read the input and write in the buffer

			if(var->ret != -1)
			{
				var->num = var->buf[0];
				if(var->num == 's') { var->sp = 1;} // if input s, signal to sm  ** peak time south passenger green during its time, otherwise will be red
				if(var->num == 'w') { var->wp = 1;} // if input w, signal to sm  ** peak time west passenger green during its time, otherwise will be red
				if(var->num == 'n') { var->np = 1;} // if input n, signal to sm  ** peak time north passenger green during its time, otherwise will be red
				if(var->num == 'e') { var->ep = 1;} // if input e, signal to sm  ** peak time east passenger green during its time, otherwise will be red
				if(var->num == 'o') { var->ot = 1;} // if input o, signal to sm  ** changes to off peak time
				if(var->num == 'p') { var->pt = 1;} // if input p, signal to sm  ** changes to peak time
															/* off peak traffic */
				if(var->num == 'a') { var->ews = 1;} // if input a, signal to sm   ** off peak ew straight green and back to ns straight green
				if(var->num == 'b') { var->ewr = 1;} // if input b, signal to sm   ** off peak ew right turn green and back to ns straight green
				if(var->num == 'c') { var->nsr = 1;} // if input c, signal to sm   ** off peak ns right turn green and back to ns straight green
				if(var->num == 'd') { var->osp = 1;} // if input d, signal to sm   ** stop ns traffic and south passenger green also ew
				if(var->num == 'f') { var->owp = 1;} // if input f, signal to sm   ** ns traffic keep going just west passenger green
				if(var->num == 'g') { var->onp = 1;} // if input g, signal to sm   ** stop ns traffic and north passenger green also ew
				if(var->num == 'h') { var->oep = 1;} // if input h, signal to sm   ** ns traffic keep going just east passenger green
															/* train approaches */
				if(var->num == 't') { var->sm1 = 0;} // if input t, signal to sm   ** train has gone, boom gate up
				if(var->num == 'u') { var->sm1 = 1;} // if input u, signal to sm   ** train has arrived at the station, boom gate down

			}
	}

}


void *tlight(void* data) {

	Mydata *var = (Mydata*)data;
	var->L1 = 0;
	var->L2 = 0;
	var->L3 = 0;
	var->L4 = 0;
	var->sp = 0;
	var->wp = 0;
	var->np = 0;
	var->ep = 0;
	var->pt = 0;
	var->ot = 0;
	var->sm1 = 0;

	int (*recieve)(server_data *);						// Cast send to function
	recieve = message_init(&incomingDataHandler);		// function registration

	char msg;

	for(int i; i < 100; i++){
			msg = recieve(&data->s_data);						// data and its length
			if(msg == 'p'){
				printf("Message Received in main: %c\n", msg);
				msg = 0;										// Clear Message
			}
			else{
				printf("Nothing Received...\n");
				msg = 0;
			}
			sleep(5);
	}

	while(1){


		switch(var->currState) {

				case state0:// All directions red

					if(var->owp == 1 || var->oep == 1){ // off peak
						if(var->oep == 1){
							var->oep = 0;
							var->currState = state18;
						}
						else{
							var->owp = 0;
							var->currState = state19;
						}

					}
					else{
							printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
							printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
							printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");

							timer_settime(var->timer_id, 0, &var->red, NULL);
							MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 1 second interval

					}

									if(var->ot == 1){

											if(var->pt == 1){
												var->ot = 0;
												var->pt = 0;
												var->currState = state1;

											}
											else if(var->onp == 1 || var->osp == 1 || var->ews == 1){

												var->currState = state18;
											}
											else if(var->ewr == 1){
												var->ewr = 0;
												var->currState = state19;

											}
											else if(var->nsr == 1){
												var->nsr = 0;
												var->currState = state20;

											}
											else{
												var->currState = state17;

											}

									}

									else{

											if(var->L2 == 1){
												var->currState = state5	;
												var->L2 = 0;
											}
											else if(var->L3 == 1){
												var->currState = state9 ;
												var->L3 = 0;
											}
											else if(var->L4 == 1){
												var->currState = state13 ;
												var->L4 = 0;
																		}
											else{
												var->currState = state1	;
											}
									}
									break;


				case state1:// L1 - East


						if(var->sp==1){  // ES-green, ER-green, EL-green, NL-green, SP-green
							printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-green \n \t\t\tNP-red \n");
							printf("WS-red \t\t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-green \nWL-red \t\t\t\t\t\tEL-green \nWP-red \t\t\t\t\t\tEP-red \n");
							printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-green \n\n");
							timer_settime(var->timer_id, 0, &var->green, NULL);
							MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


							var->sp = 0;
							}

						else{ // ES-green, ER-green, EL-green, NL-green, SP-red
							printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-green \n \t\t\tNP-red \n");
							printf("WS-red \t\t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-green \nWL-red \t\t\t\t\t\tEL-green \nWP-red \t\t\t\t\t\tEP-red \n");
							printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
							timer_settime(var->timer_id, 0, &var->green, NULL);
							MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval
							}
						var->currState = state2;
							break;

				case state2:// ES-green, ER-green, EL-yellow, NL-yellow, SP-red
					printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-yellow \n \t\t\tNP-red \n");
					printf("WS-red \t\t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-green \nWL-red \t\t\t\t\t\tEL-yellow \nWP-red \t\t\t\t\t\tEP-red \n");
					printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
					timer_settime(var->timer_id, 0, &var->yellow, NULL);
					MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval



					var->currState = state3;
							break;

				case state3: // ES-green, ER-green, EL-red, NL-red, SP-red

					printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
					printf("WS-red \t\t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-green \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
					printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
					timer_settime(var->timer_id, 0, &var->green, NULL);
					MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


					var->currState = state4;
							break;

				case state4: // ES-yellow, ER-yellow, EL-red, NL-red, SP-red

					printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
					printf("WS-red \t\t\t\t\t\tES-yellow \nWR-red \t\t\t\t\t\tER-yellow \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
					printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
					timer_settime(var->timer_id, 0, &var->yellow, NULL);
					MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


					var->L2 = 1;
					var->currState = state0;
							break;

				case state5: // L2 - South

					if(var->wp==1){  // SS-green, SR-green, SL-green, EL-green, WP-green
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-green \nWP-green \t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-green\n \t\t\tSL-green\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

						var->wp = 0;
						}

					else{ // SS-green, SR-green, SL-green, EL-green, WP-red
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-green \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-green\n \t\t\tSL-green\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

						}

					var->currState = state6;
							break;

				case state6:

					if(var->sm1 == 0){  // SS-green, SR-green, SL-yellow, EL-yellow, WP-red

						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-yellow \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-green\n \t\t\tSL-yellow\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval
						var->currState = state7;

					}
					else{  // SS-green, SR-yellow, SL-yellow, EL-yellow, WP-red
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-yellow \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-yellow\n \t\t\tSL-yellow\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-yellow\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval
						var->L3 = 1;
						var->currState = state0;
					}
					break;

				case state7:

					if(var->sm1 == 0){ // SS-green, SR-green, SL-red, EL-red, WP-red

						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-green\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

					}

						var->currState = state8;
								break;

				case state8:
					if(var->sm1 == 0){ // SS-yellow, SR-yellow, SL-red, EL-red, WP-red

						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-yellow\n \t\t\tSR-yellow\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval

					}

						var->L3 = 1;
						var->currState = state0;
							break;

				case state9: // L3 - West

						if(var->np==1){  // WS-green, WR-green, WL-green, SL-green, NP-green
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-green \n");
						printf("WS-green \t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-red \nWL-green \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-green\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

						var->np = 0;
						}

					else{ // wS-green, wR-green, wL-green, SL-green, NP-red
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-red \nWL-green \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-green\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

						}

						var->currState = state10;
							break;

				case state10: // wS-green, wR-green, wL-yellow, SL-yellow, NP-red
					if(var->sm1 == 0){
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-red \nWL-yellow \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-yellow\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval
						var->currState = state11;
					}
					else{  // jump to state 12
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-red \nWL-yellow \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-yellow\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval
						var->currState = state12;

					}
						break;

				case state11: // wS-green, wR-green, wL-red, SL-red, NP-red
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second intervall

						var->currState = state12;
						break;

				case state12: // wS-yellow, wR-yellow, wL-red, SL-red, NP-red
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-yellow \t\t\t\t\tES-red \nWR-yellow \t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval

						var->L4 = 1;

						var->currState = state0;
						break;

				case state13: // L4 - North

					if(var->ep==1){  // NS-green, NR-green, NL-green, WL-green, EP-green
						printf("\t\t\tNS-green\n \t\t\tNR-green \n \t\t\tNL-green \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-green \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-green \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second intervall

						var->ep = 0;
						}

					else{ // NS-green, NR-green, NL-green, WL-green, EP-red
						printf("\t\t\tNS-green\n \t\t\tNR-green \n \t\t\tNL-green \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-green \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second intervall
						}

						var->currState = state14;
						break;

				case state14:// NS-green, NR-green, NL-yellow, WL-yellow, EP-red
						printf("\t\t\tNS-green\n \t\t\tNR-green \n \t\t\tNL-yellow \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-yellow \t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval

						var->currState = state15;
						break;

				case state15:// NS-green, NR-green, NL-red, WL-red, EP-red
						printf("\t\t\tNS-green\n \t\t\tNR-green \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second intervall

						var->currState = state16;
						break;

				case state16:// NS-yellow, NR-yellow, NL-red, WL-red, EP-red
						printf("\t\t\tNS-yellow\n \t\t\tNR-yellow \n \t\t\tNL-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWL-red \t\t\t\t\t\tEL-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSL-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval

						var->currState = state0;
						break;

				case state17:// ns-green also ep and wp can request green
					pthread_cond_signal(&var->s);

					if(var->pt == 1 || var->onp == 1 || var->osp == 1 || var->ews == 1 || var->ewr == 1 || var->nsr == 1){
						printf("\t\t\tNS-yellow\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-yellow\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval

						var->currState = state0;
					}

					else{
						if(var->oep == 1){
							printf("\t\t\tNS-green\n \t\t\tNR-red \n \t\t\tNP-red \n");
							printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-green \n");
							printf("\t\t\tSS-green\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
							timer_settime(var->timer_id, 0, &var->green, NULL);
							MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


							var->oep = 0;

						}

						else if(var->owp == 1){
							printf("\t\t\tNS-green\n \t\t\tNR-red \n \t\t\tNP-red \n");
							printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-green \t\t\t\t\tEP-red \n");
							printf("\t\t\tSS-green\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
							timer_settime(var->timer_id, 0, &var->green, NULL);
							MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


							var->owp = 0;
						}

						else{

						printf("\t\t\tNS-green\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-green\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval

						}
					}
					break;

				case state18:// if onp == 1 || ews || osp == 1

					if(var->onp == 1){
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-green \n");
						printf("WS-green \t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-yellow \t\t\t\t\tES-yellow \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


						var->onp = 0;
						var->currState = state0;
					}
					else if (var->ews == 1){

						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-yellow \t\t\t\t\tES-yellow \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


	 					var->ews = 0;
						var->currState = state0;
					}
					else{
						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-green \t\t\t\t\tES-green \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-green \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-yellow \t\t\t\t\tES-yellow \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


						var->osp = 0;
						var->currState = state0;

					}
						break;

				case state19:// if ewr == 1


						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-green \t\t\t\t\tER-green \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


						printf("\t\t\tNS-red\n \t\t\tNR-red \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-yellow \t\t\t\t\tER-yellow \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-red\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


						var->onp = 0;
						var->currState = state0;
						break;

				case state20:// if nsr == 1

						printf("\t\t\tNS-red\n \t\t\tNR-green \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-green\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->green, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 3 second interval


						printf("\t\t\tNS-red\n \t\t\tNR-yellow \n \t\t\tNP-red \n");
						printf("WS-red \t\t\t\t\t\tES-red \nWR-red \t\t\t\t\t\tER-red \nWP-red \t\t\t\t\t\tEP-red \n");
						printf("\t\t\tSS-red\n \t\t\tSR-yellow\n \t\t\tSP-red \n\n");
						timer_settime(var->timer_id, 0, &var->yellow, NULL);
						MsgReceive(var->chid, &msg, sizeof(msg), NULL); // 2 second interval


						var->currState = state0;
						break;

		}

	}
}

int main(void)
{
	anyname data;
	data.s_data = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0};
	data.m_data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	pthread_t th1, th2, th3, th4;

	struct sched_param th1_param, th2_param, th3_param;
	pthread_attr_t th1_attr, th2_attr, th3_attr;

	pthread_attr_init (&th1_attr);
	pthread_attr_init (&th2_attr);
	pthread_attr_init (&th3_attr);


	pthread_attr_setschedpolicy (&th1_attr, SCHED_RR);  //
	pthread_attr_setschedpolicy (&th2_attr, SCHED_RR);	// 	Scheduled round robin
	pthread_attr_setschedpolicy (&th3_attr, SCHED_RR);  //

	th1_param.sched_priority = 5;
	th2_param.sched_priority = 3;
	th3_param.sched_priority = 5;

	pthread_attr_setschedparam (&th1_attr, &th1_param);
	pthread_attr_setschedparam (&th2_attr, &th2_param);
	pthread_attr_setschedparam (&th3_attr, &th3_param);
	pthread_attr_setinheritsched (&th1_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched (&th2_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched (&th3_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize (&th1_attr, 8000);
	pthread_attr_setstacksize (&th2_attr, 8000);
	pthread_attr_setstacksize (&th3_attr, 8000);


	pthread_create(&th1,&th1_attr,tlight,&data); // thread 1 created
	pthread_create(&th2,&th2_attr,sensor,&data.m_data); // thread 2 created
	pthread_create(&th3,&th3_attr,timer,&data.m_data);  // thread 3 created
	pthread_create(&th4,NULL,server,&data.s_data);

	pthread_join(th1,NULL);
	pthread_join(th2,NULL);
	pthread_join(th3,NULL);


	return 0;
}



