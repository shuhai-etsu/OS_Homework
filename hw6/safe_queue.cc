/*---------------------------------------------
This program demonstrates safe queue that has internal lock and semaphore 
File name: safe_queue.cc
Author: Shuhai Li
Date: April 24,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/
#include <pthread.h>
#include <semaphore.h>
#include <queue>

#ifndef SAFE_QUEUE_h
#include "safe_queue.h"
#endif

#ifndef PRODUCT_RECORD_h
#include "product_record.h"
#endif


/*-------------Default constructor--------------------*/
safe_queue::safe_queue()
{

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("error in mutex initialization\n");
	}
	
	if (pthread_mutex_init(&lock2, NULL) != 0)
	{
		printf("error in mutex initialization\n");
	}
	
	if(sem_init(&sem, 0, 0) != 0)
	{
		printf("error in semaphore initialization\n");
	}
	
};

/*-------------enqueue method--------------------*/
void safe_queue::enqueue(product_record pr)
{
	pthread_mutex_lock(&lock);      //lock this section code
	myqueue.push(pr);               //add the record to the queue
	sem_post(&sem);                  //signal a record was added
	pthread_mutex_unlock(&lock);    //unlock 
};

/*-------------dequeue method--------------------*/
product_record safe_queue::dequeue()
{
	product_record pr;                 
	pthread_mutex_lock(&lock2);         //lock the following code
	while (myqueue.empty()==true)
	{
		sem_wait(&sem);
	}
	pr=myqueue.front();                //read the record at the front of the queue
	myqueue.pop();			           //remove the record at the front of the queue	
	pthread_mutex_unlock(&lock2);      //unlock the code
	return pr;
};