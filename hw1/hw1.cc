#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include "product_record.h"
using namespace std;
int main(int argc, char *argv[])
{
        int NumberOfRecord=3;
        product_record record[NumberOfRecord];
        int i,j;
        string in=argv[1];
        string out=argv[2];
        ifstream inClientFile(in.c_str(),ios::in);
	ofstream outClientFile(out.c_str(),ios::out);       
        //Read records from the input file
        if (!inClientFile)
	{
		cerr<<"File could not be opened"<<endl;
		exit(1);
	}
    
	for (j=0;j<NumberOfRecord;j++)
        {
           inClientFile>>record[j].idnumber>>record[j].name>>record[j].price>>record[j].number>>record[j].tax>>record[j].sANDh>>record[j].total;
           for (i=0;i<MAXSTAGES;i++)
           {
              inClientFile>>record[j].stations[i];
           }
        }
        inClientFile.close();//Close the file
        
        //Display records on screen
        for (j=0;j<NumberOfRecord;j++)
        {
           cout.precision(2);
           cout<<"IDNumber:        "<<record[j].idnumber<<endl;
           cout<<"Name:            "<<record[j].name<<endl;
           cout<<"Price($):        "<<fixed<<record[j].price<<endl;
           cout<<"Quantity:        "<<record[j].number<<endl;
           cout<<"Tax($):          "<<record[j].tax<<endl;
           cout<<"S&H($):          "<<record[j].sANDh<<endl;
           cout<<"Total($):        "<<record[j].total<<endl;
           cout<<"Process stations:";
           for (i=0;i<MAXSTAGES;i++)
           {
               cout<<record[j].stations[i]<<" ";
           }   
           cout<<"\n\n";
           cout<<"Press ENTER to continue\n";
           cin.get();
        }
        cout<<"All records have been displayed\n";

        //Output records into a file
        for (j=0;j<NumberOfRecord;j++)
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
	 cout<<"All records have been save to 'outfile1'\n";
         outClientFile.close();
         return 0;
}
