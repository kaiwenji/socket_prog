#include <iostream> 
#include<fstream>
#include<string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include<vector>
using namespace std;
int main(int argc, char** argv) {
	/**
	open the text file.
	**/
	ifstream out;
	out.open(argv[1],ios::in);
	if(!out.is_open())
	{
		cout<<"file open failed";<<endl;
		return 0;
	}
	cout<<"The client is up and running."<<endl;

	/**
	create a new TCP socket and bind with the specific address
	**/	
	int count=0;
	struct in_addr addr;
	ulong l;
	l=inet_addr("127.0.0.1");
	memcpy(&addr,&l,4);
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(23749);
	server.sin_addr=addr;
	memset(&(server.sin_zero), 0, 8);	
	int socketNum=socket(AF_INET,SOCK_STREAM,0);
	
	if(connect(socketNum,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1)
	  {
	    cout<<"bad connection"<<endl;
	    return 0;
	  }
	
	
	/**
	ready to get rows from the target file
	sending 10 rows one time and then wait for 1 second before next tranferring. If less than 10 rows, then just sending all of them.
	**/
	char tmp[50];
	char message[10]="@"; //this is the signal message for stopping sending row
	char sbuffer[500];	//sending buffer
	int len=0;
	int rowNum=0;
	while(out.getline(tmp,50))
	{
		bool wrong_row=false;
		rowNum++;
		for(int i=0;i<strlen(tmp);i++)
		{
			if(tmp[i]==' ')
			{
				cout<<"input error in row "<<rowNum+1<<": SPACE is not allowed"<<endl;
				wrong_row=true;
				break;
			}
		}
		if(wrong_row)
		{
			continue;
		}
		if(strlen(tmp)==0)

	    {
	      continue;
	    }
	  
		if(count%10==0)
	    {
			send(socketNum,sbuffer,len,0);
			len=0;
			sleep(1);
			cout<<"..."<<endl;	//signal letting user wait for 1 second
	    }
		for(int i=0;i<strlen(tmp);i++)
	    {
			sbuffer[len++]=tmp[i];
	    }
		count++;
	}
	if(len!=0)
	  {
	    send(socketNum,sbuffer,len,0);
	    sleep(1);
	    cout<<"..."<<endl;
	  }
	send(socketNum,message,10,0); 		//send the stop message
	cout<<"The client has successfully finished sending ";
	cout<<count;
	cout<<" lines to the edge server."<<endl;
	
	
	/**
	receiving step
	the receiving is also ten rows one time. after receiving one message, it is splited and stored in a list 
	**/
	vector<char*> list;
	vector<int> length;
	char buffer[500];	//receiving buffer
	while(1)
	{
	    int len=recv(socketNum,buffer,500,0);
		if(len>0)
		{
			if(buffer[0]=='@')	//if get stop message , run out of the loop
			{
				break;
			}
			bool flag=false;
			char * tmp=new char[20];
			int cou=0;
			for(int i=0;i<=len;i++)
			{
				
				if(buffer[i]=='$')
				{
					if(cou>0)
					{
						char* newRow=new char[20];
						flag=false;
						for(int j=0;j<cou;j++)
						{
							newRow[j]=tmp[j];
						}
						list.push_back(newRow);
						length.push_back(cou);
						cou=0;
						if(i==len)
						{
							break;
						}
					}
				}				
				if(!flag)
				{
					if(buffer[i]=='=')
					{
						flag=true;
						i++;
					}
				}
				else
				{
					tmp[cou++]=buffer[i];
				}
				
				//cout<<buffer[i];
			}
			if(buffer[len-1]=='@')
			{
				break;
			}
			cout<<"..."<<endl;
			delete tmp;
		}
	}
	cout<<"The client has successfully finished receiving all computation resultsfrom the edge server."<<endl;
	cout<<"The final computation result are:"<<endl;
	
	
	/**
	output the result
	**/
	for(int i=0;i<list.size();i++)
	  {
	    for(int j=0;j<length[i];j++)
	      {
		cout<<list[i][j];
	      }
	    cout<<endl;
	  }
	  
	  /**
	  release cache and close the socket.
	  **/
	close(socketNum);
	
	for(int i=0;i<list.size();i++)
	  {
	    delete list[i];
	  }
	
	out.close();
	return 0;
	
}
