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
#include<errno.h>
using namespace std;
void result_return(vector<char*>,int);
void getBuffer(vector<char*>&,vector<int>&,char*,int,int&);
int udp(sockaddr_in,sockaddr_in,vector<char*>,vector<int>,int);
int main() 
{
	/**
	parameter for all addresses that will be used
	**/
	struct in_addr addr;
	ulong l;
	l=inet_addr("127.0.0.1");
	memcpy(&addr,&l,4);
	struct sockaddr_in client;
	struct sockaddr_in server;
	struct sockaddr_in edge;
	struct sockaddr_in back_and;
	struct sockaddr_in back_or;
	server.sin_family=AF_INET;
	server.sin_port=htons(23749);
	server.sin_addr=addr;
	memset(&(server.sin_zero), 0, 8);
 	edge.sin_family=AF_INET;
	edge.sin_port=htons(24749);
	edge.sin_addr=addr;
	memset(&(edge.sin_zero), 0, 8);
 	back_and.sin_family=AF_INET;
	back_and.sin_port=htons(22749);
	back_and.sin_addr=addr;
	memset(&(back_and.sin_zero), 0, 8);
	back_or.sin_family=AF_INET;
	back_or.sin_port=htons(21749);
	back_or.sin_addr=addr;
	memset(&(back_and.sin_zero), 0, 8);
	
	/**
	create a TCP socket and bind it with the address
	**/
	int socketNum=socket(PF_INET,SOCK_STREAM,0);
	if(bind(socketNum,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1)
	  {
	    cout<<"bad binding"<<endl;
	    cout<<errno<<endl;
	    return 0;
	  }

	if(listen(socketNum,5)==-1)
	  {
	    cout<<"bad Listen"<<endl;
	    return 0;
	  }
	int clientSize=sizeof(struct sockaddr_in);
	cout<<"The edge server is up and running."<<endl;

	/**
	This is the out loop for the TCP service
	**/
	while(1)
	{
	    int childSocket=accept(socketNum,(struct sockaddr *)&client,(socklen_t*)&clientSize);
	    if(childSocket==-1)
		{
			cout<<"accept error"<<endl;
			return 0;
		}

		int number=0;	//the number of rows received
		vector<char*> list;	//storing for rows
		vector<int> length; //storing for the length of each row
		char buffer[500];	//buffer for receiving
		memset(buffer,0,500);
		
		/**
		this is second loop for receiving the message from client(ten or less than ten rows a time). This loop is ended by receiving a '@'.
		Even if there is no task for one backend server, there also will be one end message sended to this backend server.
		**/
		while(true)
		{
		    int cnt=recv(childSocket,buffer,500,0);
		    if(cnt>0)
		    {
		      
				if(buffer[0]=='@')
				{
					
						/**
						TCP receiving is ended. Next, UDP step is started.
						**/
					  //udp connection
					char udpbuffer[50];
					memset(udpbuffer,0,50);
					bool flag=false;
					cout<<"The edge server has received ";
					cout<<list.size();
					cout<<" lines from the client using TCP over port 23749."<<endl;
					  
					  /**
					  create a UDP socket.
					  **/
					int udpSock=socket(AF_INET,SOCK_DGRAM,0);
					
					//set TIMEOUT
					struct timeval interval;
					interval.tv_sec=3;
					interval.tv_usec=0;
					if (setsockopt(udpSock, SOL_SOCKET, SO_RCVTIMEO, &interval, sizeof(interval)) < 0) 
					{  
						cout<<"TIMEOUT is not supported"<<endl;  
					}  
					if(udpSock==-1)
					{
						cout<<"socket failed"<<endl;
					}
					if(bind(udpSock,(struct sockaddr*)&edge,sizeof(struct sockaddr))==-1)
					{
						cout<<"bad binding"<<endl;
					}
					  
					  /**
					  ready to send rows from list to those two backand server
					  **/
					udp(back_and,back_or,list,length,udpSock);		    		    
					cout<<"The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP overport 24749."<<endl;
					cout<<"The computation results are:"<<endl;
				  
				  /**
				  receiving result from those two backend server
				  this is the third loop for receiving rows from UDP one by one. this step is ended when two end messages("@") 
				  from two backand servers are comming together. Even is no row will be processed in one server, there will be one end message comes. 
				  **/
				  //udp data exchange
					while(1)
					{
						int len=recv(udpSock,udpbuffer,50,0);
						if(len<0)
						{
							cout<<"failure in recv"<<endl;
							return 0;
						}
						
						if(len>0)
						{
						
							if(udpbuffer[0]=='@')	//when there are two end messages have came
							{
							
								if(!flag)
								{
									flag=true;
									continue;
								}
								cout<<"The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server And."<<endl;
								break;
							}
					
							/**
							parse the arrived row. The format of the row is  result information + "#" + the number of the row + "$"
							for example: 1 and 1 = 1#0$   
							**/
							int p=0,index=0;
							char* tmp=new char[50];
							/**
							get the buffer information
							**/
							for(int i=0;i<len;i++)
								{
									if(udpbuffer[i]=='#')
									{
										p=i;
									}
									tmp[i]=udpbuffer[i];
								}
						  
						  
							/**
							get the number of this row
							**/
							index=tmp[p+1]-'0';
							if(tmp[p+2]!='$')
							{
								index+=10*(udpbuffer[p+2]-'0');
							}
							delete list[index];
							
							/**
							output the result
							**/
							for(int i=0;i<len;i++)
							{
								if(tmp[i]=='#')
								{
									break;
								}
								cout<<tmp[i];
							}
							cout<<endl;
							list[index]=tmp;
							memset(udpbuffer,0,50);
				
						}
					}
					close(udpSock);
			  
				  /**
				  step for return result to client
				  **/
					result_return(list,childSocket);
					cout<<"The edge server has successfully finished sending all computation results to the client."<<endl;
				}
		      
		      
				else
				{ 
				/**
				if not get the end message, continuely ready to get row from client.
				**/
					getBuffer(list,length,buffer,cnt,number);
					memset(buffer,0,500);
				}
		    }
		    

			//error check
		    else
			{
				if(cnt<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))
				{
					continue;
				}
				else
				{
					close(childSocket);
					break;
				}
			}
		}
			


		//release cache
		
		for(int i=0;i<list.size();i++)
		{
			delete list[i];
		}
	    close(childSocket);
	
	}
	close(socketNum);
}


/**
function for sending back information to client
sending information ten rows one time. if less than ten rows, it send all result one time.
**/
void result_return(vector<char*> list, int childSocket)
{
	char tmp[500];
	int len=0;
	char message[10]="@";
	for(int i=0;i<list.size();i++)
	{
		if(i%10==0&&len!=0)
		{
			send(childSocket,tmp,len,0);
			len=0;
			sleep(1);	//after sending ten rows, sleep for 1 second.
			cout<<"..."<<endl; 	//message for pause.
		}
		int l=0;
		for(;list[i][l]!='#';l++)
		{
			tmp[len++]=list[i][l];
		}
		tmp[len++]='$';
	}
	
	//send the last one 
	
	if(len>0)
	{
		send(childSocket,tmp,len,0);
		sleep(1);
		cout<<"..."<<endl;
	}
	send(childSocket,message,10,0);
}


/**
function for receving information from client
At this step, I also add the index of the row to the message plus a ending signal "$"
**/
void getBuffer(vector<char*> &list, vector<int> &length, char* buffer, int cnt,int &number)
{
   char* tmp=new char[50];
   int tmp_num=number;
   int i=0,count=0;
   for(;i<=cnt;i++)
    {
		if(((buffer[i]=='a'||buffer[i]=='o')&&i!=0)||(i==cnt))
		{
			tmp_num=number;
			number++;
			tmp[count++]='#';
	   
	   
			//add the index. if the index is 8 then '8' will add to the message. If the index is "23" then '32' will add to the message.
			// the index is stored in reverse pattern which is easier for operation. After all, it only at most has two digits.
			if(tmp_num==0)
			{
				tmp[count++]=(char)('0'+tmp_num);
			}
			else
			{
				while(tmp_num>0)
				{
					tmp[count++]=(char)('0'+(tmp_num%10));
					tmp_num/=10;
				}
			}
	   
			tmp[count++]='$';
			char *newRow=new char[50];
			for(int j=0;j<count;j++)
			{
				newRow[j]=tmp[j];
			}
			list.push_back(newRow);	//add to the list for storing
			length.push_back(count);		//add the length of this row
			count=0;
	   
		}
		if(i<cnt)
		{
			tmp[count++]=buffer[i];
		}
    }
	delete tmp;
}
  
/**
function for sending information to two backend servers
**/
int udp(sockaddr_in  back_and, sockaddr_in back_or,vector<char*> list,vector<int> length,int udpSock)
{
	char sucMessage[10]="@";
	int and_count=0,or_count=0;
	int len=0;
	for(int i=0;i<list.size();i++)
    {
		if(list[i][0]=='a') // if the row is an and operation
		{
			len=length[i];
			and_count++;
			
			if(sendto(udpSock,list[i],len,0,(struct sockaddr*)&back_and,sizeof(struct sockaddr))==-1)
			{
				cout<<"failure in sending"<<endl;
				return 0;
			}
		}
		if(list[i][0]=='o')	//if the row is an or operation
		{
			len=length[i];
			or_count++;
			if(sendto(udpSock,list[i],len,0,(struct sockaddr*)&back_or,sizeof(struct sockaddr))==-1)
			{
				cout<<"failure in sending"<<endl;
				return 0;
			}
		}
    }
	//send the end message
	sendto(udpSock,sucMessage,10,0,(struct sockaddr*)&back_and,sizeof(struct sockaddr));
	sendto(udpSock,sucMessage,10,0,(struct sockaddr*)&back_or,sizeof(struct sockaddr));
	cout<<"The edge has successfully sent ";
	cout<<or_count;
	cout<<" lines to Backend-Server OR."<<endl;
	cout<<"The edge has successfully sent ";
	cout<<and_count;
	cout<<" lines to Backend-Server AND."<<endl;
	return 0;
}
