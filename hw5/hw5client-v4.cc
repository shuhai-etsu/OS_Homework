/*---------------------------------------------
This program hw5client.cc is the code for client side.
It demonstrate use of socket in network programming 
Author: Shuhai Li
Date: April 10,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <strings.h>

#ifndef PRODUCT_RECORD_h
#include "product_record.h"
#endif

using namespace std;

//Function definition
void * pthread_read_record(void* arg);   //function to read record from input file
void * pthread_write_record(void* arg);  //function to write records to output file
void WriteFile(ofstream& outClientFile, vector<product_record>& record);
void MakeAllDoneRecord(product_record &pr);      //Generate a termination signal by producing a record with id=-1

int sockdesc;                            //socket descriptor

int main(int argc, char** argv)
{
	struct addrinfo *myinfo; // Address record
	char hostname[81];       //host name
	char portnum[81];        //port number
	int connection;          //file descriptor of connection
	product_record pr;       //product record
	pthread_t p[2];           //thread identifiers
	int rc;                 //return value of thread_create call

	if ( argc <5 )         //need to specify four arguments 
	{
	  cout << "five parameters required" << endl;
	  cout << "Example: hw5 infile1 outfile einstein.etsu.edu 3600"<<endl;
	  exit(0);
	}
	
	if ( argc == 5 )
	{
	  strcpy(hostname, argv[3]);    //copy the third argument to hostname
	  strcpy(portnum, argv[4] );    //copy the fourth argument to port number
	}
	else
	{
	  strcpy(portnum, "2000");      // if port number is not specified, use default port number 2000
	}
	
   //Create a socket
   cout << "Before socket" << endl;   
   sockdesc = socket(AF_INET, SOCK_STREAM, 0);  
   if ( sockdesc < 0 )
   {
      cout << "Error creating socket" << endl;
      exit(0);
   }

   // Set up the address record
   cout << "Before getaddrinfo" << endl;
   if ( getaddrinfo(hostname, portnum, NULL, &myinfo) != 0 )
   {
      cout << "Error getting address" << endl;
      exit(0);
   }

   // Connect to the host
   cout << "Before connect" << endl;
   connection = connect(sockdesc, myinfo->ai_addr, myinfo->ai_addrlen);
   if ( connection < 0 )
   {
      cout << "Error in connect" << endl;
      exit(0);
   }
   cout << "Client connection = " << connection << endl;
   
	/* ---------Create threads---------------------*/
	for (int i = 0; i < 2; i++)
	{
       switch (i)
	   {
		   case 0:
		   {
		   	   rc=pthread_create(&p[i], NULL,pthread_read_record, (void *) argv[1]);   //run the thread to read records from input file
			   if(rc!=0)
	           {
		           printf("ERROR;return code from pthread_create() is %d\n",rc);
	           }
			   break;			   
		   }
			
			case 1:
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
	for (int i = 0; i < 2; i++)
	{
        pthread_join(p[i], NULL);                 //join two threads after they've done their jobs
	}
	
   close(sockdesc);
   return 0;

}

/*--------------first thread----------------------
main function is to read records from a file, send records to server. 
It also generates an all-done record to signal the end of process---------*/
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

	   write(sockdesc, (char*)&pr, sizeof(pr));
	}
	inClientFile.close();                   //Close the file
	MakeAllDoneRecord(pr);                 //Make an all-done record
	write(sockdesc, (char*)&pr, sizeof(pr));
	cout<<"The first thread read "<<num_record<< " product records  from input file"<<endl;
    pthread_exit(NULL);
}


/*------------Thread to read records from server after records processed-----------------*/
void * pthread_write_record(void* arg)
{
	int num_record=0;
	int value;
	product_record pr;   //declare product_record struct 
	pr.idnumber=1;
	vector<product_record> record_processed(0);    //define vector container that contains product_record struct
	string out=(char *) arg;
	ofstream outClientFile(out.c_str(),ios::out);  //instantiate an ofstream object   
  	do
	{
		value = read(sockdesc, (char*)&pr, sizeof(pr));
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
