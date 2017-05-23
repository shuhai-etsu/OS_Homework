/*---------------------------------------------
This program demonstrate process creation using fork() 
Author: Shuhai Li
Date: Febrary 11,2016
Course: Operating System
Organization: East Tennessee State University
------------------------------------------------*/
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "product_record.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

//Define Pipe
int mypipe0[2];
int mypipe1[2];
int mypipe2[2];
int mypipe3[2];
int mypipe4[2];
int mypipe5[2];
int mypipe6[2];

product_record pr;
product_record pr_out;

//Function definition

void child(int i);
void station0();
void station1();
void station2();
void station3();
void station4();
void ReadFile(ifstream& inClientFile, vector<product_record>& record);
void WriteFile(ofstream& outClientFile, vector<product_record>& record);
void DisplaySingleRecord(product_record & item);
void MakeAllDoneRecord();

int main(int argc, char *argv[])
{

	int i;
	int j=0;
	int n = 5;
	int number_of_record=0;
	pid_t pids[5];
    pipe(mypipe0);
	pipe(mypipe1);
	pipe(mypipe2);
    pipe(mypipe3);
	pipe(mypipe4);
	pipe(mypipe5);	
	pipe(mypipe6);
/*---------Read records from input file -----------------*/
	string in=argv[1];                             //assign the second argument of main function to string in
	string out=argv[2];                            //assign the third argument of main function to string out
	ifstream inClientFile(in.c_str(),ios::in);     //instantiate an ifstream object
	ofstream outClientFile(out.c_str(),ios::out);  //instantiate an ofstream object     
	vector<product_record> record(0);              //define vector container that contains product_record struct 
	vector<product_record> record_processed(0);    //define vector container that contains product_record struct 
	ReadFile(inClientFile, record);                //call ReadFile function	
    cout<<record.size()<<endl;
/* ---------Start five children processes------------*/
	for (i = 0; i < n; ++i)
	{
	 if ((pids[i] = fork()) < 0)
		{
		perror("fork");
		abort();
		}  
		else if (pids[i] == 0)
		{
		child(i);
		//exit(0);
		}
	}
	
		while (j<record.size())
		{
			pr=record[j];
			close(mypipe0[0]);
			write(mypipe0[1],(char*) &pr,sizeof(product_record));	
			number_of_record++;
			close(mypipe5[1]);
			read(mypipe5[0],(char*) &pr,sizeof(product_record));
			DisplaySingleRecord(pr); 
			if(pr.idnumber!=-1)	
			{
				record_processed.push_back(pr);
			}	
			j++;
		}
/*-------Make and send an all-done record----------------------*/	
    MakeAllDoneRecord();
	close(mypipe0[0]);
	write(mypipe0[1],(char*) &pr,sizeof(product_record));	
			
	close(mypipe5[1]);
	read(mypipe5[0],(char*) &pr,sizeof(product_record));
	// if(pr.idnumber!=-1)	
	// {
		// record_processed.push_back(pr);
	// }	
/*------ Wait for children to exit--------------- */
	int status;
	pid_t pid;
	while (n > 0) 
	{
		pid = wait(&status);
		//printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
		--n;  
	}
	cout<<"Parent processed"<<number_of_record<< "product records"<<endl;
/*--------Write records to a file---------------------*/
	WriteFile(outClientFile, record_processed);              //call WriteFile function
	return 1;
}

void child(int i)
{
	switch (i)
	{
		case 0:
		   station0();
		   break;
		case 1:
		   station1();
		   break;
		case 2:
            station2();
            break;
        case 3:
            station3();
            break;
        case 4:
            station4();
            break;			
	}
}

//station0 process
void station0()
{
    int i=0;
    do
	{
	   close(mypipe0[1]);
	   read(mypipe0[0],(char*) &pr,sizeof(product_record));
	    //DisplaySingleRecord(pr); 
        pr.tax=pr.price*pr.number*0.05;
		pr.stations[0]=1;
		if (pr.number>=1000)
		{
			close(mypipe6[0]);
			write(mypipe6[1],(char*) &pr,sizeof(product_record));
		} else
		{
			close(mypipe1[0]);
			write(mypipe1[1],(char*) &pr,sizeof(product_record));	
		}
		if(pr.idnumber!=-1)
        i++;  

	}while (pr.idnumber!=-1);

	cout<<"Station 0 processed"<<i<< "product records"<<endl;
	exit(0);
}

//station1 process
void station1()
{
	int i=0;
	do
	{
		close(mypipe1[1]);
		read(mypipe1[0],(char*) &pr,sizeof(product_record));
		//DisplaySingleRecord(pr); 
        if (pr.price*pr.number<1000)
		{
			pr.sANDh=pr.price*pr.number*0.01+10;   
		}else
		{
			pr.sANDh=0;
		}
		
	    pr.stations[1]=1; 
		close(mypipe2[0]);
		write(mypipe2[1],(char*) &pr,sizeof(product_record));
		if(pr.idnumber!=-1)
		i++;
		//cout<<"process1 executed\n";
	}while (pr.idnumber!=-1);
	cout<<"Station 1 processed"<<i<< "product records"<<endl;
	exit(0);
}

//station2 process
void station2()
{
	int i=0;
	int fds[2];
	do
	{
        // fds[0]=mypipe6[0];
		// fds[1]=mypipe2[0];
		// select(2,&fds,NULL,NULL,NULL);
		// close(mypipe6[1]);
		// read(mypipe6[0],(char*) &pr,sizeof(product_record))
		// cout<<"read from station0"<<endl;

		close(mypipe2[1]);
		read(mypipe2[0],(char*) &pr,sizeof(product_record));
		cout<<"read from station1"<<endl;

		

		//DisplaySingleRecord(pr); 
        pr.total=pr.price*pr.number+pr.tax+pr.sANDh;
        pr.stations[2]=1;		
		close(mypipe3[0]);
		write(mypipe3[1],(char*) &pr,sizeof(product_record));
		
		if(pr.idnumber!=-1)
		i++;
	}while (pr.idnumber!=-1);
	cout<<"Station 2 processed"<<i<< "product records"<<endl;
	exit(0);
}
 
//station3 process 
void station3()
{
    int i=0;
	double running_total=0;
	do
	{
		close(mypipe3[1]);
		read(mypipe3[0],(char*) &pr,sizeof(product_record));
        //DisplaySingleRecord(pr);

		running_total=running_total+pr.total;
		cout.setf(ios::fixed);
	    cout.setf(ios::showpoint);
		cout.precision(2);

        cout<<"Running total so far is:$"<<running_total<<endl;	
        pr.stations[3]=1;		
		close(mypipe4[0]);
		write(mypipe4[1],(char*) &pr,sizeof(product_record));
		if(pr.idnumber!=-1)
		i++;
	} while (pr.idnumber!=-1);
	cout<<"Station 3 processed"<<i<< "product records"<<endl;
	exit(0);
}
 
//station4 process 
void station4()
{
	int i=0; 
	do 
	{
		close(mypipe4[1]);
		read(mypipe4[0],(char*) &pr,sizeof(product_record));
		pr.stations[4]=1;	
		if(pr.idnumber!=-1)
		DisplaySingleRecord(pr);
		close(mypipe5[0]);
		write(mypipe5[1],(char*) &pr,sizeof(product_record));
		if(pr.idnumber!=-1)
		i++;
	} while (pr.idnumber!=-1);
	cout<<"Station 4 processed"<<i<< "product records"<<endl;
	exit(0);
}

/*-------------------ReadFile function implementation----------------------*/
void ReadFile(ifstream& inClientFile, vector<product_record>& record)
{
	int i;                                   //iteration variable
	product_record data_item;                //a single product_record object

	if (!inClientFile)                      //Check if the file can be opened
	{
		cerr<<"File could not be opened"<<endl;  //if can not be opened, write out an error message
		exit(1);
	}
    
	while(inClientFile.eof()==0)              //Check if the end of file is reached, if not keep reading data from the file
    {
	   inClientFile>>data_item.idnumber>>data_item.name>>data_item.price>>data_item.number>>data_item.tax>>data_item.sANDh>>data_item.total;
	    
	   for (i=0;i<MAXSTAGES;i++)
	   {
		  inClientFile>>data_item.stations[i];
	   }	
	   record.push_back(data_item);          //add the record to the vector container
	}
	inClientFile.close();                   //Close the file
}

/*--------------------WriteFile function---------------------*/
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
		 cout<<"All records have been saved\n";	
	}
	 outClientFile.close();
}

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

void MakeAllDoneRecord()
{
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