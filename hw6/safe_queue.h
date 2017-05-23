/*---------------------------------------------
This program demonstrates safe queue that has internal lock and semaphore 
File name: safe_queue.h
Author: Shuhai Li
Date: April 24,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/
#include <pthread.h>
#include <semaphore.h>
#include <queue>

#ifndef PRODUCT_RECORD_h
#include "product_record.h"
#endif

class safe_queue
{
	private:
	    queue<product_record> myqueue;    // queue containing product records
		pthread_mutex_t lock;             //lock used in enqueue method
		pthread_mutex_t lock2;            //lock used in dequeue method
		sem_t sem;                        //semaphore 
		
	public:
		safe_queue();	                 //default constructor
		void enqueue(product_record pr);  //add a record to the queue. Parameter type is product_record
		product_record dequeue();         //remove a record from the queue. Return a product_record

};