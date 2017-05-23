/*---------------------------------------------
This program demonstrates Pthread programming techniques and thread synchronization 
Author: Shuhai Li
Date: March 24,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>

//product record struc definition
#ifndef PRODUCT_RECORD_h
#include "product_record.h"
#endif
using namespace std;
//Function definition
void * pthread_read_record(void* arg);   //function to read record from input file
void * pthread_func0(void*);            //function for station 0: calculating tax
void * pthread_func1(void*);            //function for station 1:calculating shiping and handling
void * pthread_func2(void*);            //function for station 2:calculating total
void * pthread_func3(void*);            //function for station 3:calculating running total
void * pthread_func4(void*);            //function for station 4:displaying record
void * pthread_write_record(void* arg);  //function to write records to output file

void WriteFile(ofstream& outClientFile, vector<product_record>& record);
void DisplaySingleRecord(product_record &item);  //Display a product record on terminal
void MakeAllDoneRecord(product_record &pr);      //Generate a termination signal by producing a record with id=-1

//define six queues as shared data between threads
queue<product_record> queue0;
queue<product_record> queue1;
queue<product_record> queue2;
queue<product_record> queue3;
queue<product_record> queue4;
queue<product_record> queue5;

//define six mutex locks, corresponding to each queue
pthread_mutex_t lock0=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5=PTHREAD_MUTEX_INITIALIZER;

//define six semaphores to avoid busy waiting of threads
sem_t sem0;
sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t sem4;
sem_t sem5;


/*-----------------main function ------------------------------*/
int main(int argc, char *argv[])
{
    
	int num_thread = 7;       //number of threads 
    pthread_t p[num_thread];  //thread identifiers
	int rc;                 //return value of thread_create call
	sem_init(&sem0,0,0);    //initialize six semaphores
	sem_init(&sem1,0,0);
	sem_init(&sem2,0,0);
	sem_init(&sem3,0,0);
	sem_init(&sem4,0,0);
	sem_init(&sem5,0,0);

	
	/*---------check for number of arguments -----------------*/
    if(argc != 3) { 
      cout << "two arguments required for running the program";
      return -1;
    }
	
	/* ---------Create seven threads---------------------*/
	for (int i = 0; i < num_thread; i++)
	{
       switch (i)
	   {
		   case 0:
		   {
		   	   rc=pthread_create(&p[i], NULL,pthread_read_record, (void *) argv[1]);   //run the thread to write records
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;			   
		   }

			case 1:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_func0, NULL);
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);   //run the thread to calculate tax
	           }
			   break;			
			}
			
			case 2:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_func1, NULL);                //run the thread to calculate tax
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;		
			}
			
			case 3:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_func2, NULL);                 //run the thread to calculate shipping and handling cost
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;		
			}
			
			case 4:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_func3, NULL);                 //run the thread to calculate total of a record
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;		
			}
			
			case 5:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_func4, NULL);           //run the thread to sum up the running totoal of processed records
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;			
			}
			
			case 6:
			{
		   	   rc=pthread_create(&p[i], NULL,pthread_write_record, (void *) argv[2]);    //run the thread to write processed records to output file
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
	
	sem_destroy(&sem0);                          //destroy six semaphores
	sem_destroy(&sem1);
	sem_destroy(&sem2);
	sem_destroy(&sem3);
	sem_destroy(&sem4);
	sem_destroy(&sem5);
	
	pthread_mutex_destroy(&lock0);            //desctroy pthread_mutex objects
	pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	pthread_mutex_destroy(&lock3);
	pthread_mutex_destroy(&lock4);
	pthread_mutex_destroy(&lock5);

	return 0;
}

/*--------------first thread----------------------
----------main function is to read records and generate all-done record---------*/
void * pthread_read_record(void * arg)
{
	int num_record=0;
	string in;
	in=(char *) arg;
	product_record pr;                          //a single product_record object
	ifstream inClientFile(in.c_str(),ios::in);     //instantiate an ifstream object

	if (!inClientFile)                      //Check if the file can be opened
	{
		cerr<<"File can not be opened!"<<endl;  //if can not be opened, write out an error message
		exit(1);
	}
	if (inClientFile.peek()==inClientFile.eof())
	{
		cerr<<"File is empty!"<<endl;
		exit(1);
	}
	
	while(!inClientFile.eof())              //Check if the end of file is reached, if not keep reading data from the file
    {
	   inClientFile>>pr.idnumber
	               >>pr.name
				   >>pr.price
				   >>pr.number
				   >>pr.tax
				   >>pr.sANDh
				   >>pr.total;
	    
	   for (int i=0;i<MAXSTAGES;i++)
	   {
		  inClientFile>>pr.stations[i];
	   }
	   num_record++;               //increment the number of records processed
	   
       pthread_mutex_lock(&lock0);//lock the queue
	   queue0.push(pr);          //add the record to the vector container
	   pthread_mutex_unlock(&lock0); //unlock the queue
	   
	   sem_post(&sem0);
	}
	inClientFile.close();                   //Close the file
	
	MakeAllDoneRecord(pr);                 //Make an all-done record
	
    pthread_mutex_lock(&lock0);            //lock queue0
	queue0.push(pr);                       //push the record to queue0
	pthread_mutex_unlock(&lock0);          //unlock queue
	sem_post(&sem0);                       //signal sem0 associated threads they can run
	
	cout<<"The first thread read "<<num_record<< " product records  from input file"<<endl;
    pthread_exit(NULL);
}


/*--------------Station 0 thread----------------------
----------main function is to calculate tax---------*/
void * pthread_func0(void*)
{
    int num_record=0;                     //set the number of processed record to zero
	product_record pr;
	pr.idnumber=1;                        //set idnumber=1 to void error in while(pr.idnumber!=-1) if queue0 is empty in the first run
    do
	{
		sem_wait(&sem0);                  //wait signal from sem0 associated threads 
		pthread_mutex_lock(&lock0);  //lock queue0
		pr=queue0.front();           //get the front record from queue0
		queue0.pop();                //get rid of the front record from queue0
		pthread_mutex_unlock(&lock0); //unlock queue0
		
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
void * pthread_func4(void*)
{
	product_record pr;
	pr.idnumber=1;
	int num_record=0; 
	
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
		pthread_mutex_lock(&lock5);      //lock queue 5
		queue5.push(pr);
		pthread_mutex_unlock(&lock5);    //unlock queue 5		
		sem_post(&sem5);                    //signal sem5 associated threads

	} while (pr.idnumber!=-1);
	cout<<"Station 4 processed "<<num_record<< " product records"<<endl;

	pthread_exit(NULL);
}

void * pthread_write_record(void* arg)
{
	int num_record=0;
	product_record pr;   //declare product_record struct 
	pr.idnumber=1;
	vector<product_record> record_processed(0);    //define vector container that contains product_record struct
	string out=(char *) arg;
	ofstream outClientFile(out.c_str(),ios::out);  //instantiate an ofstream object   
  	do
	{
		sem_wait(&sem5);                         //wait for signal from sem5 associated threads
		pthread_mutex_lock(&lock5);         //lock queue 5
		pr=queue5.front();
		queue5.pop();
		pthread_mutex_unlock(&lock5);      //unlock queue 5
		if(pr.idnumber!=-1)	                                    //if not the all-done record, add the record to a record collection
		{
			record_processed.push_back(pr);
			num_record++;
		}			
	}while (pr.idnumber!=-1);

    WriteFile(outClientFile, record_processed); 
	cout<<"Last thread wrote "<<num_record<< " product records to the external file"<<endl;
	pthread_exit(NULL);
}


/*--------------------WriteFile function---------------------
---write product record collection to an external file-------*/
void WriteFile(ofstream& outClientFile, vector<product_record>& record)
{
	int i,j;                                    //iteration variables
	if (record.size()==0)                      //check if there is any records in the container
	{
		cout<<"There is no reocrds to save\n";
	}
	else
	{
		for (j=0;j<record.size();j++)          //write the records from the vector container into a file
		{
			outClientFile << record[j].idnumber<<endl;
			outClientFile << record[j].name<<endl;
			outClientFile << record[j].price<<endl;
			outClientFile << record[j].number<<endl;
			outClientFile << record[j].tax<<endl;
			outClientFile << record[j].sANDh<<endl;
			outClientFile << record[j].total<<endl;
			for (i=0;i<MAXSTAGES;i++)
			{
				outClientFile << record[j].stations[i]<<" ";
			}
			outClientFile <<"\n";
		 }           
	}
	 outClientFile.close();
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

/*--------------------MakeAllDoneRecord function--------------------
------------------------make a fake record with idnumber of -1 to
--------------------signal child processes of end of processing -------*/
void MakeAllDoneRecord(product_record & pr)
{
	//product_record pr;
	pr.idnumber=-1;
	pr.name[0]='b';
	pr.name[1]='r';
	pr.name[2]='e';
	pr.name[3]='a';
	pr.name[4]='d';
	
	pr.price=10;
	pr.number=2;
	pr.tax=0;
	pr.sANDh=0;
	pr.total=0;
	pr.stations[0]=0;
	pr.stations[1]=0;
	pr.stations[2]=0;
	pr.stations[3]=0;
	pr.stations[4]=0;
}