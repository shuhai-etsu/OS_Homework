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
using namespace std;

void child_function();                       //definition of child_function

int main()
{
    int return_value = fork();                 //call system call fork()
    int status;
/*-------error occurred-----------------*/
	if (return_value < 0)                             
	{
		fprintf(stderr, "fork failed");
		return 1;
	}
/*------start child process------------*/
	if (return_value == 0)                           
    {
        child_function();                      //call child_function() to start child process
    } 
/*------start parent process------------*/
	if (return_value > 0)
    {
        wait(&status);                          //parent process waits for child process to complete
		system("clear");                        //clear the console for output records in the next step
        system("more outfile1");               //display the records in outfile1 on screen
    } 

    return 0;
}

/*---------------Child process implementation------------------*/
void child_function()
{
  cout << "Child process ID  is: " << getpid() << endl;      //print child process ID
  cout << "Parent process ID is: " << getppid() << endl;     //print parent process ID
  
  if (access("hw1", F_OK) == 0 )                    // if hw1 exists
  {
    char* const args[] = {(char*) "hw1", (char*) "infile1", (char*) "outfile1", (char*) 0}; //define arguments for hw1
    execv("hw1", args);                            //execute hw1
  } 
  else                                            // hw1 doesn't exist, print out an error message
  {
    cerr << "hw1 does not exist!";
  }
  exit(0); 
}