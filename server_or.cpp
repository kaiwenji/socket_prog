#include <iostream> 
#include<fstream>
#include<string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include<vector>
#include<math.h>
using namespace std;
char* dispose(char*,int);
/**
server_or is for getting tasks from edge server, doing or operations and send the result back to edge server
**/
int main() {
	//store the information of addresses
	struct in_addr addr;
	ulong l;
	l=inet_addr("127.0.0.1");
	memcpy(&addr,&l,4);
	struct sockaddr_in remote;
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(21749);
	local.sin_addr=addr;
	remote.sin_family=AF_INET;
	remote.sin_port=htons(24749);
	remote.sin_addr=addr;
	memset(&(local.sin_zero), 0, 8);
	int count=0;
	
	//create a UDP socket
	int sock=socket(PF_INET,SOCK_DGRAM,0);
	if(bind(sock,(struct sockaddr*)&local,sizeof(struct sockaddr))==-1)
	{
		cout<<"bad binding"<<endl;
		return 0;
	}
	cout<<"The Server OR is up and running using UDP on port 21749."<<endl;
	cout<<"The Server OR start receiving lines from the edge server for OR computation. The computation results are:"<<endl;
	
	//the loop is for data transfer and task operations
	//first step: get information from edge server and do operations
	//second step: store the result in a list
	//third step: send back the result together to edge server.
	while(1)
	{
	    socklen_t clientSize=sizeof(struct sockaddr_in);
	    vector<char*> result_list;
	    char buffer[50];
	    char message[50];
	    char success[10]="@";
	    int len;
	    while(1)
		{
			//receiving the information from edge server one by one.
			len=recvfrom(sock,buffer,50,0,(struct sockaddr *)&remote,&clientSize);	
			if(len==-1)  
			{  
				cout<<"recv error"<<endl;   
				return 0;  
			}
			if(len>0)
			{
				//end receiving because of the '@' end message
				if(buffer[0]=='@')
				{
					cout<<"The Server OR has successfully received ";
					cout<<count;
					cout<<" lines from the edge server and finished all OR computations"<<endl;
					count=0;
					break;
				}
				//after receiving one, do the operation and put the result to list.
				char* res=dispose(buffer,len);
				result_list.push_back(res);
				count++;
			}
		}
		
		
		//send back the result to edge server.
	    for(int i=0;i<result_list.size();i++)
		{
			char* res=result_list[i];
			int length=0;
			while(res[length]!='$')
			{
				length++;
			}
			if(len=sendto(sock,res,length+1,0,(struct sockaddr*)&remote,sizeof(struct sockaddr))==-1)
			{
				cout<<"sending error"<<endl;
				return 0;
			}
		
		}
	    if(len=sendto(sock,success,10,0,(struct sockaddr*)&remote,sizeof(struct sockaddr))==-1)
		{
			cout<<"sending error"<<endl;
			return 0;
		}
		cout<<"The Server OR has successfully finished sending all computation results to the edge server."<<endl;  
	    
		//release cache
		for(int i=0;i<result_list.size();i++)
	      
		{
			delete result_list[i];
		}

	
	}
	close(sock);
	cout<<close<<endl;
}

/**
function for doing operation for each row.
**/
char* dispose(char* source,int length)
{
	//get the message from buffer
	char * test=new char[50];
	for(int i=0;i<length;i++)
	{
		test[i]=source[i];
	}
	
	//get the position from both two operands
	int lena=0,lenb=0,len=0,start=0;
	char num[2];
	bool digiFlag=false;
	for(int i=0;test[i]!='#';i++)
	{
		len=i;
		if(test[i]==',')
		{
			if(start!=0)
			{
				lena=i-start-1;
			}
			start=i;
		}
	}
	num[0]=test[len+2];
	if(test[len+3]!='$')
	{
		digiFlag=true;
		num[1]=test[len+3];
	}
	lenb=len-start;
	
	//store the two operands
	len=max(lena,lenb);
	char a[len],b[len];
	start=0;
	int count=0;
	for(int i=0;i<length;i++)
	{
		if(test[i]==',')
		{
			start++;
			if(start==1)
			{
				for(int j=0;j<len-lena;j++)
				{
					a[j]='0';
				}
				count=len-lena;
			}
			if(start==2)
			{
				for(int j=0;j<len-lenb;j++)
				{
					b[j]='0';
				}
				count=len-lenb;
			}
		}
		else
		{
			if(start==1)
			{
				a[count++]=test[i];
			}
			if(start==2)
			{
				b[count++]=test[i];
			}
		}
	}
	delete test;
	
	//do the operations
	char* res=new char[50];
	bool flag=false;
	count=0;
	for(int i=len-lena;i<len;i++)
	{
		res[count++]=a[i];
	}
	res[count++]=' ';
	res[count++]='o';
	res[count++]='r';
	res[count++]=' ';
	for(int i=len-lenb;i<len;i++)
	{
		res[count++]=b[i];
	}
	res[count++]=' ';
	res[count++]='=';
	res[count++]=' ';
	for(int i=0;i<len;i++)
	{
		int tmp=(a[i]-'0')|(b[i]-'0');
		if(flag||tmp==1)
		{
		  flag=true;
		  res[count++]=(char)(tmp+'0');
		}
	}
	if(!flag)
	{
		res[count++]='0';
	}
	res[count++]='#';
	res[count++]=num[0];
	if(digiFlag)
	{
		res[count++]=num[1];
	}
	res[count]='$';
	for(int i=0;res[i]!='#';i++)
	{
		cout<<res[i];
	}
	cout<<endl;

	return res;
}
