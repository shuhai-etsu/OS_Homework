/*---------------------------------------------
This program hw5client.cc is code for the server.
It demonstrate use of socket in network programming 
Author: Shuhai Li
Date: April 10,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/

#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

#ifndef PRODUCT_RECORD_h
#include "product_record.h"
#endif
using namespace std;

//Function declaration
void * pthread_func0(void* arg);            //function for station 0: calculating tax
void * pthread_func1(void*);                //function for station 1:calculating shiping and handling
void * pthread_func2(void*);                //function for station 2:calculating total
void * pthread_func3(void*);                //function for station 3:calculating running total
void * pthread_func4(void* arg);            //function for station 4:displaying record
void DisplaySingleRecord(product_record &item);  //Display a product record on terminal

//define queues as shared data between threads
queue<product_record> queue1;
queue<product_record> queue2;
queue<product_record> queue3;
queue<product_record> queue4;

//define mutex locks, corresponding to each queue
pthread_mutex_t lock1=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4=PTHREAD_MUTEX_INITIALIZER;

//define semaphores to avoid busy waiting of threads
sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t sem4;

int main(int argc, char** argv )
{
	int sockdesc;            // Socket descriptor
	struct addrinfo *myinfo; // Address record
	char portnum[81];
	//product_record pr;
	int connection;
	//int value;
	int num_thread = 5;       //number of threads 
	pthread_t p[num_thread];  //thread identifiers
	int rc;                 //return value of thread_create call

	sem_init(&sem1,0,0);
	sem_init(&sem2,0,0);
	sem_init(&sem3,0,0);
	sem_init(&sem4,0,0);

   // Create the socket with domain, type, protocol as internet, stream, default 
   sockdesc = socket(AF_INET, SOCK_STREAM, 0);
   if ( sockdesc < 0 )
   {
      cout << "Error creating socket" << endl;
      exit(0);
   }

   // Get the port number from the command line
   if ( argc > 1 )
   {
      strcpy(portnum, argv[1]);
   }
   else
   {
      strcpy(portnum, "2000");
   }

   // Set up the address record
   if ( getaddrinfo("0.0.0.0", portnum, NULL, &myinfo) != 0 )
   {
      cout << "Error getting address" << endl;
      exit(0);
   }

   // Bind the socket to an address
   if (bind(sockdesc, myinfo->ai_addr, myinfo->ai_addrlen) < 0 )
   {
      cout << "Error binding to socket" << endl;
      exit(0);
   }

   // Now listen to the socket
   if ( listen(sockdesc, 1) < 0 )
   {
      cout << "Error in listen" << endl;
      exit(0);
   }

   // Accept connections
	cout << "Calling accept" << endl;
	connection = accept(sockdesc, NULL, NULL);  //accept connection request from clients
	if ( connection < 0 )                       // check the returned descriptor
	{
		cout << "Error in accept" << endl;
		exit(0);
	}
	else
	{
		for (int i = 0; i < num_thread; i++)      //Generate five threads to process records separately
		{
		   switch (i)
		   {
				case 0:
				{
				   rc=pthread_create(&p[i], NULL,pthread_func0, (void *) connection);
				   if(rc!=0)
				   {
					   printf("ERROR;return code from pthread_create() is %d\n",rc);   //run the thread to calculate tax
				   }
				   break;			
				}
				
				case 1:
				{
				   rc=pthread_create(&p[i], NULL,pthread_func1, NULL);                //run the thread to calculate tax
				   if(rc!=0)
				   {
					   printf("ERROR;return code from pthread_create() is %d\n",rc);
				   }
				   break;		
				}
				
				case 2:
				{
				   rc=pthread_create(&p[i], NULL,pthread_func2, NULL);                 //run the thread to calculate shipping and handling cost
				   if(rc!=0)
				   {
					   printf("ERROR;return code from pthread_create() is %d\n",rc);
				   }
				   break;		
				}
				
				case 3:
				{
				   rc=pthread_create(&p[i], NULL,pthread_func3, NULL);                 //run the thread to calculate total of a record
				   if(rc!=0)
				   {
					   printf("ERROR;return code from pthread_create() is %d\n",rc);
				   }
				   break;		
				}
				
				case 4:
				{
				   rc=pthread_create(&p[i], NULL,pthread_func4, (void *) connection);           //run the thread to sum up the running totoal of processed records
				   if(rc!=0)
				   {
					   printf("ERROR;return code from pthread_create() is %d\n",rc);
				   }
				   break;			
				}
				
		   } //switch(i)
		}//for (int i = 0; i < num_thread; i++)

		/*------ Wait for thread to join to exit--------------- */
		for (int i = 0; i < num_thread; i++)
		{
			pthread_join(p[i], NULL);                 //join seven threads after they'done their jobs
		}  

	} //if ( connection < 0 )

   close(connection);      // Close the connection
   return 0;

} // main( )


/*--------------Station 0 thread----------------------
----------main function is to calculate tax---------*/
void * pthread_func0(void* arg)
{
    int num_record=0;                     //set the number of processed record to zero
	product_record pr;
	pr.idnumber=1;                        //set idnumber=1 to void error in while(pr.idnumber!=-1) if queue0 is empty in the first run
	int connection=(int) arg;
	int value;
    do
	{
		value = read(connection, (char*)&pr, sizeof(pr));  //read a record from client
		
		pr.tax=pr.price*pr.number*0.05;  //calculate tax
		pr.stations[0]=1;                //set station byte to zero indicating it has been processed
		
		if (pr.number>=1000)     //if the quatity of an order geater than 1000, set sANDh to zero and send it to station2 for processing
		{
			pthread_mutex_lock(&lock2); //lock queue2
			pr.sANDh=0;
			queue2.push(pr);              //add to queue2
			pthread_mutex_unlock(&lock2); //unlock queue2
			sem_post(&sem2);              //signal sem2 associated process that they can go ahead do stuff
		} 
		else
		{
			pthread_mutex_lock(&lock1);  //if order quantity less than 1000, lock queue1
			queue1.push(pr);             //send record to station 1
			pthread_mutex_unlock(&lock1); //unlock queue1
			sem_post(&sem1);              //signal sem1 associated process they can go ahead
		}
		
		if(pr.idnumber!=-1)
		num_record++;                    //increment the number of processed records		
	}while (pr.idnumber!=-1);
	cout<<"Station 0 processed "<<num_record<< " product records"<<endl;
	pthread_exit(NULL);
}

/*--------------Station 1 thread -------------------
----------calculate shipping and handling costs---------*/
void * pthread_func1(void*)
{
	product_record pr;
	pr.idnumber=1;
	int num_record=0;
	do
	{
        sem_wait(&sem1);                     //wait for signal from sem1 asscociated threads
		pthread_mutex_lock(&lock1);          //lock queue1 for reading and removing record from the queue
		pr=queue1.front();
		queue1.pop();
		pthread_mutex_unlock(&lock1);       //release the lock
		
		pr.sANDh=pr.price*pr.number*0.01+10;   //calculate shipping and handling (1% of order+$10)
		pr.stations[1]=1;                      //flip the flag in station array to indicate processing
		if(pr.idnumber!=-1)
		num_record++;                         //increment of the number of records processed
	
		pthread_mutex_lock(&lock2);           //lock queue2 for adding record
		queue2.push(pr);
		pthread_mutex_unlock(&lock2);		//release the lock
		sem_post(&sem2);                        //signal other sem2 associated threads to run

	}while (pr.idnumber!=-1);
	cout<<"Station 1 processed "<<num_record<< " product records"<<endl;
	pthread_exit(NULL);
}

/*--------------Station 2 thread-------------------
----------calculate total costs---------------------*/
void * pthread_func2(void*)
{
	int num_record=0;                        //number of records processed by station 2
    product_record pr;
	pr.idnumber=1;
	do
	{
		sem_wait(&sem2);                      //wait for signal from sem2 assocated threads
		pthread_mutex_lock(&lock2);       //lock queue2
		pr=queue2.front();
		queue2.pop();
		pthread_mutex_unlock(&lock2);     //unlock queue2
		
		pr.total=pr.price*pr.number+pr.tax+pr.sANDh;          //sum up order value, tax and shipping and handling costs to get total cost
		pr.stations[2]=1;		                              //flip flag to indidcate record processed by station 2
		if(pr.idnumber!=-1)                                
			num_record++;                                        //increment number of records processed
		
		pthread_mutex_lock(&lock3);         //lock queue 3
		queue3.push(pr);
		pthread_mutex_unlock(&lock3);        //unlock queue 3		
		sem_post(&sem3);              //signal sem3 associated threads
		
	}while (pr.idnumber!=-1);
	cout<<"Station 2 processed "<<num_record<< " product records"<<endl;
}

/*--------------Station 3 thread-------------------
----------display running total costs------------------*/
void * pthread_func3(void*)
{
	product_record pr;
	pr.idnumber=1;
    int num_record=0;                                     //number of records processed by station 3
	double running_total=0;                    //running total of records processed by station 3
	do
	{
        sem_wait(&sem3);                     //wait for signal from sem3 associated threads
		pthread_mutex_lock(&lock3);        //lock queue 3
		pr=queue3.front();
		queue3.pop();
		pthread_mutex_unlock(&lock3);      //unlock queue 3
		
		running_total=running_total+pr.total;       //calculate total running cost
		cout.setf(ios::fixed);
		cout.setf(ios::showpoint);
		cout.precision(2);
		if(pr.idnumber!=-1)
		cout<<"Running total so far is:$"<<running_total<<endl;	   //display total running cost on console 
		pr.stations[3]=1;		                                //indicate the record has been processed by station 3
		if(pr.idnumber!=-1)
		num_record++;                                       //increment number of record processed
	
		pthread_mutex_lock(&lock4);          //lock queue4
		queue4.push(pr);                     //push record to queue 4
		pthread_mutex_unlock(&lock4);        //unlock queue 4		
		sem_post(&sem4);
                                       
	} while (pr.idnumber!=-1);
	cout<<"Station 3 processed "<<num_record<< " product records"<<endl;
	pthread_exit(NULL);
}

/*--------------Station 4 thread-------------------
----------------display record------------------*/
void * pthread_func4(void* arg)
{
	product_record pr;
	pr.idnumber=1;
	int num_record=0; 
	int connection;
	connection=(int) arg;
	do 
	{
		sem_wait(&sem4);                  //wait for to-go signal from sem4 associated threads
		pthread_mutex_lock(&lock4);   //lock queue 4
		pr=queue4.front();
		queue4.pop();
		pthread_mutex_unlock(&lock4);   //unlock queue 4
		
		pr.stations[4]=1;	                                 //indicate record has been processed	
		if(pr.idnumber!=-1)
		{
			DisplaySingleRecord(pr);                          //display record on console
			num_record++;                            //increment number of records processed by station 4
		}

		write(connection, (char*)&pr, sizeof(pr));  //write record back to client

	} while (pr.idnumber!=-1);
	cout<<"Station 4 processed "<<num_record<< " product records"<<endl;

	pthread_exit(NULL);
}

/*--------------------DisplaySingleRecord function--------------------
------------------------display record information on console -------*/
void DisplaySingleRecord(product_record &item)
{
   cout.setf(ios::fixed);
   cout.setf(ios::showpoint);
   cout.precision(2);
   cout<<"IDNumber:        "<<item.idnumber<<endl;
   cout<<"Name:            "<<item.name<<endl;
   cout<<"Price($):        "<<fixed<<item.price<<endl;
   cout<<"Quantity:        "<<item.number<<endl;
   cout<<"Tax($):          "<<item.tax<<endl;
   cout<<"S&H($):          "<<item.sANDh<<endl;
   cout<<"Total($):        "<<item.total<<endl;
   cout<<"Process stations:";
   for (int i=0;i<MAXSTAGES;i++)
   {
	   cout<<item.stations[i]<<" ";
   }   
   cout<<endl;
   cout<<"---------------------------\n"<<endl;
}