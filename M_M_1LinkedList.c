/*  starts 3/25/2017

    AVBee rev.6.0 Last Update : April 16,2017
    Modified Queue M/M/1 with linkedlist

    https://github.com/AVBee/M-M-1_LINKEDLIST
    
    This is homework's program used for educational purpose only.
    The program was originally written not by myself. I modified it with 
    additional function and implemented Linked List method that was described 
    in 'README.md' file. 
    
*/

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "lcgrand.h" /* Header file for random-number generator. */
#include <stdlib.h>

#define Q_LIMIT 100 /* Limit on queue length. */
#define BUSY 1 /* Mnemonics for server's being busy */
#define IDLE 0 /* and idle. */

int next_event_type,  num_delays_required, num_events,
server_status, num_app;

unsigned int num_custs_delayed;

int num_in_q;

float area_num_in_q, area_server_status, mean_interarrival, mean_service,
 sim_time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3],
 total_of_delays, v_sysdelay, v_systim, syst_delay_var, v_sysleng, v_servrate, v_arvrate ;

FILE *infile, *outfile;

struct test_struct *head = NULL;
struct test_struct *curr = NULL;

struct test_struct
{
    struct test_struct *next;
};

// build struct for initialize node
struct test_struct* create_list()
{
  //  printf(" as [%d]\n",val);
    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    //ptr->val = val;
    ptr->next = NULL;

    head = curr = ptr;

    return ptr;

}

//struct for append node
struct test_struct* add_to_list( bool add_to_end)
{
    if(NULL == head)
    {
        return (create_list());
        num_in_q++;
    }


    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
   // ptr->val = val;
    ptr->next = NULL;

    if(add_to_end)
    {
        curr->next = ptr;
        curr = ptr;
    }
    else
    {
        ptr->next = head;
        head = ptr;
    }
    num_in_q++;
    return ptr;

}

//struct from removal of node*
struct test_struct* remove_list()
{

    struct test_struct *ptr = head;
    struct test_struct *t;

    if(ptr == NULL) // SAFETY FUNCTION
    {
       printf("error");
    }

    while(ptr->next != NULL)
    {

       t = ptr;
       ptr = ptr->next;

    }
    free(t->next);
    t->next=NULL;
    num_in_q--;
    return ptr;

}

void initialize(void);
void timing(void);
void arrive(void);
void depart(void);
void report(void);
void update_time_avg_stats(void);
void mean_delay(void);
void serv_util(void);
void mean_qu_length(void);
void syst_leng(void);
float expon(float mean);
void mean_sys_length(void);


int main() /* Main function. */
{
    int i = 0, ret = 0;
    struct test_struct *ptr = NULL;

	/* Open input and output files. */
	infile = fopen("mmc.in", "r");
	outfile = fopen("mm1.out", "w");

	/* Specify the number of events for the timing function. */
	num_events = 2;

	/* Read input parameters. */
	fscanf(infile, "%f %f %d", &mean_interarrival, &mean_service,
	&num_delays_required);

	v_arvrate = 1/mean_service;
	mean_service = v_arvrate;

	v_servrate = 1/mean_interarrival;
	mean_interarrival = v_servrate;

	/* Write report heading and input parameters. */
	fprintf(outfile, "Single-server queueing system\n\n");
	fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n",
	mean_interarrival);
	fprintf(outfile, "Mean service time%16.3f minutes\n\n", mean_service);
	fprintf(outfile, "Number of customers%14d\n\n", num_delays_required);
	/* Initialize the simulation. */
	initialize();
	/* Run the simulation while more delays are still needed. */

	while (num_custs_delayed < num_delays_required)
	{

		/* Determine the next event. */
		timing();

		/* Update time-average statistical accumulators. */
		update_time_avg_stats();


 /* Invoke the appropriate event function. */
	 switch (next_event_type) {
		 case 1:
		 arrive();
		 break;
		 case 2:
		 depart();
		 break;
		 }
	}
		/* Invoke the report generator and end the simulation. */
	 report();
    // print_aku();  /* #DEBUG */
	 fclose(infile);
	 fclose(outfile);
	 return 0;
}

void initialize(void) /* Initialization function. */
{

	/* Initialize the simulation clock. */
	sim_time = 0.0;


	/* Initialize the state variables. */
	server_status = IDLE;
	num_in_q = 0;
	time_last_event = 0.0;

	/* Initialize the statistical counters. */
	num_custs_delayed = 0;
	total_of_delays = 0.0;
	area_num_in_q = 0.0;
	area_server_status = 0.0;

	/* Initialize event list. Since no customers are present, the departure
	(service completion) event is eliminated from consideration. */

	time_next_event[1] = sim_time + expon(mean_interarrival);

	time_next_event[2] = 1.0e+30;
}

void timing(void) /* Timing function. */
{
	int i;
	float min_time_next_event = 1.0e+29;
	next_event_type = 0;

	/* Determine the event type of the next event to occur. */
	for (i = 1; i <= num_events; ++i)
		if (time_next_event[i] < min_time_next_event)
            {
                min_time_next_event = time_next_event[i];
                next_event_type = i;
            }

 /* Check to see whether the event list is empty. */
     if (next_event_type == 0)
     {
     /* The event list is empty, so stop the simulation. */
         fprintf(outfile, "\nEvent list empty at time %f", sim_time);
         exit(1);
     }


 /* The event list is not empty, so advance the simulation clock. */
    sim_time = min_time_next_event;
}
void arrive(void) /* Arrival event function. */
{
	 float delay;


	 /* Schedule next arrival. */
	 time_next_event[1] = sim_time + expon(mean_interarrival);

	 /* Check to see whether server is busy. */
	 if (server_status == BUSY)
        {
	 /* Server is busy, so increment number of customers in queue. */
            add_to_list(true);

        //print_list();

	 /* Check to see whether an overflow condition exists. */
        if (num_in_q > Q_LIMIT)
            {
         /* The queue has overflowed, so stop the simulation. */

                 fprintf(outfile, "\nOverflow of the array time_arrival at");
                 fprintf(outfile, " time %f", sim_time);
                 exit(2);
            }
	 /* There is still room in the queue, so store the time of arrival of the
	 arriving customer at the (new) end of time_arrival. */
        time_arrival[num_in_q] = sim_time;
        }
	 else
        {
	 /* Server is idle, so arriving customer has a delay of zero. (The
	 following two statements are for program clarity and do not affect
	 the results of the simulation.) */
             delay = 0.0;
             total_of_delays += delay;
             /* Increment the number of customers delayed, and make server busy. */
             ++num_custs_delayed;
             server_status = BUSY;

             /* Schedule a d eparture (service c
            void mean_queue_length(void)ompletion). */
             time_next_event[2] = sim_time + expon(mean_service);
         }
}
void depart(void) /* Departure event function. */
{
	 int i;
	 float delay;

	 /* Check to see whether the queue is empty. */
	 if (num_in_q == 0) {
	 /* The queue is empty so make the server idle and eliminate the
	 departure (service completion) event from consideration. */
	 server_status = IDLE;
	 time_next_event[2] = 1.0e+30;
	 }
	 else {
	 /* The queue is nonempty, so decrement the number of customers in
	 queue. */

     remove_list();

     //   print_aku(); /* #DEBUG */

	 /* Compute the delay of the customer who is beginning service and update
	 the total delay accumulator. */
	 delay = sim_time - time_arrival[1];
	 total_of_delays += delay;
	 /* Increment the number of customers delayed, and schedule departure. */
	 ++num_custs_delayed;
	 time_next_event[2] = sim_time + expon(mean_service);
	 /* Move each customer in queue (if any) up one place. */
	 for (i = 1; i <= num_in_q; ++i)
	 time_arrival[i] = time_arrival[i + 1];
	 }
}
void report(void) /* Report generator function. */
{
 /* Compute and write estimates of desired measures of performance. */

    mean_delay();

    serv_util();

    sys_del();

    mean_sys_length();

    mean_qu_length();

}

void update_time_avg_stats(void) /* Update area accumulators for time-average
 statistics. */
{
		float time_since_last_event;
		/* Compute time since last event, and update last-event-time marker. */
		time_since_last_event = sim_time - time_last_event;
		time_last_event = sim_time;
		/* Update area under number-in-queue function. */
		area_num_in_q += num_in_q * time_since_last_event;

		/* Update area under server-busy indicator function. */
		area_server_status += server_status * time_since_last_event;
	}
	float expon(float mean) /* Exponential variate generation function. */
	{
		/* Return an exponential random variate with mean "mean". */

		return -mean * log(lcgrand(1));
}

void mean_delay(void)
{
    float x;
    x = total_of_delays / num_custs_delayed;

    fprintf(outfile, "\n\nAverage delay in queue%11.3f minutes\n\n", x);

}
//Mean number of customers in the system.
//mean system length
void sys_del(void)
{
    v_sysdelay = area_num_in_q / sim_time;
    syst_delay_var = v_sysdelay + mean_service;
    fprintf(outfile, "mean system delay  %.3f \n\n", syst_delay_var);
}

void mean_qu_length(void)
{
    v_sysdelay = area_num_in_q / sim_time;
    fprintf(outfile, "mean queue length%.3f \n\n", v_sysdelay);
}
void syst_leng(void)
{

    float y, cal_temp;

    y = num_in_q / sim_time;

    fprintf(outfile, "mean queue length = %d\n\n", y);

}

void serv_util(void)
{
    float y;
    y = area_server_status / sim_time;
    fprintf(outfile, "Server utilization%15.3f\n\n", y);
}

void mean_sys_length(void)
{
    float z;
    float f = 1.0;
    z = num_delays_required/sim_time;
    fprintf(outfile, "Time simulation ended%12.3f minutes\n\n", z);
    fprintf(outfile, "mean system length %f \n\n", (num_custs_delayed + num_in_q)*f/sim_time);
}

/* THIS CODE BELOW ARE USED TO DEBUG ONLY*/


//
//void print_aku(void)
//{
//    int count=0;
//    struct test_struct *ptr = head;
//
//    printf("\n -------Printing list Start------- \n");
//    while(ptr != NULL)
//    {
//        //printf("\n [%d] \n",ptr->val);
//        ptr = ptr->next;
//        //num_in_q++;
//        count++;
//
//    }
//    printf("\n -------There are %d node------- \n", num_in_q);
//    printf("\n -------Printing list End  %d------- \n", count);
//
//  //  return num_in_q;
//}
//
//


//void AppendNode(void) {
//
//    //int i=0;
//    unsigned int count;
//
//    struct Node* temp =
//		(Node*)malloc(sizeof(Node));
//
//
//    temp->next = NULL;
//    head = temp;
//
//	if(head !=NULL)
//
//        temp->next=head;
//        head=temp;
//        printf(head);
//
//}
//void printDon(Node *head)
//{
//
//
//    //Node* cur = head;
//
//	while(head->next != NULL)
//    {
//
//        head = head->next;
//        count = 1;
//
//    }
//    printf("%d", count);
//}



//void ReleaseNode(void) {
//	struct Node* temp = front;
//	if(front == NULL) {
//		printf("Queue is Empty\n");
//		return;
//	}
//	if(front == rear) {
//		front = rear = NULL;
//	}
//	else {
//		front = front->next;
//	}
//	free(temp);
//}


//create queue
//create queue
